#!/usr/bin/env bash
# Subscribe to Tessie monitor topic; forward alarm-like lines to Mattermost incoming webhook.
# Requires: MATTERMOST_WEBHOOK_URL (e.g. from systemd EnvironmentFile=), mosquitto_sub, curl,
#           and either python3 (default on Raspberry Pi OS) or jq for JSON encoding.
# Optional: MATTERMOST_USERNAME — shown as the poster name in Mattermost (set e.g. to coldbox02 on that host).
# Logs to stderr -> visible in: journalctl -u mqtt-to-mattermost -f

set -uo pipefail

if [[ -z "${MATTERMOST_WEBHOOK_URL:-}" ]]; then
  echo "mqtt-to-mattermost: MATTERMOST_WEBHOOK_URL is unset" >&2
  exit 1
fi
for cmd in mosquitto_sub curl; do
  if ! command -v "$cmd" >/dev/null 2>&1; then
    echo "mqtt-to-mattermost: missing command: $cmd" >&2
    exit 1
  fi
done
if command -v jq >/dev/null 2>&1; then
  JSON_ENCODER=jq
elif command -v python3 >/dev/null 2>&1; then
  JSON_ENCODER=python3
else
  echo "mqtt-to-mattermost: need jq or python3 for JSON encoding (try: sudo apt install -y jq)" >&2
  exit 1
fi

# Optional: MATTERMOST_USERNAME — Mattermost incoming webhook JSON field "username" (display name).
# Set per host in /etc/mqtt-to-mattermost.env (no quotes needed: MATTERMOST_USERNAME=coldbox02)

json_payload() {
  local msg="$1"
  if [[ "$JSON_ENCODER" == jq ]]; then
    if [[ -n "${MATTERMOST_USERNAME:-}" ]]; then
      # --arg avoids jq -Rs quirks where "username" did not reliably appear in the payload
      jq -n --arg text "$msg" --arg username "$MATTERMOST_USERNAME" \
        '{text: $text, username: $username}'
    else
      jq -n --arg text "$msg" '{text: $text}'
    fi
  else
    TEXT="$msg" MATTERMOST_USERNAME="${MATTERMOST_USERNAME:-}" python3 -c '
import json, os
text = os.environ.get("TEXT", "")
u = os.environ.get("MATTERMOST_USERNAME", "").strip()
d = {"text": text}
if u:
    d["username"] = u
print(json.dumps(d), end="")
'
  fi
}

echo "mqtt-to-mattermost: listening on monTessie (localhost), JSON via $JSON_ENCODER" >&2
if [[ -n "${MATTERMOST_USERNAME:-}" ]]; then
  echo "mqtt-to-mattermost: MATTERMOST_USERNAME is set to '${MATTERMOST_USERNAME}' (Mattermost may ignore if server disables webhook overrides)" >&2
else
  echo "mqtt-to-mattermost: MATTERMOST_USERNAME unset (posts use integration default name)" >&2
fi

# Do NOT use mosquitto_sub -N here: without a trailing newline, `read` never completes a line
# and this loop never runs (MQTT still delivers to other clients message-by-message).
mosquitto_sub -h localhost -t monTessie | while IFS= read -r line; do
  case "$line" in
    *"==ALARM=="*|*"==ERROR=="*)
      echo "mqtt-to-mattermost: match, forwarding (${#line} chars)" >&2
      payload=$(json_payload "$line") || { echo "mqtt-to-mattermost: JSON encode failed" >&2; continue; }
      if ! code=$(curl -sS -o /tmp/mm-mqtt.curl.body -w '%{http_code}' \
          -X POST -H 'Content-Type: application/json' \
          -d "$payload" \
          "$MATTERMOST_WEBHOOK_URL"); then
        echo "mqtt-to-mattermost: curl failed (network / TLS / bad URL?)" >&2
        continue
      fi
      if [[ "$code" != "200" && "$code" != "201" ]]; then
        echo "mqtt-to-mattermost: Mattermost HTTP $code body: $(cat /tmp/mm-mqtt.curl.body 2>/dev/null | tr -d '\r' | head -c 500)" >&2
      else
        echo "mqtt-to-mattermost: Mattermost OK ($code)" >&2
      fi
      ;;
  esac
done
