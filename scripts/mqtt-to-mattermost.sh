#!/usr/bin/env bash
# Subscribe to Tessie monitor topic; forward alarm-like lines to Mattermost incoming webhook.
# Requires: MATTERMOST_WEBHOOK_URL (e.g. from systemd EnvironmentFile=), mosquitto_sub, curl,
#           and either python3 (default on Raspberry Pi OS) or jq for JSON encoding.
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

json_payload() {
  if [[ "$JSON_ENCODER" == jq ]]; then
    printf '%s' "$1" | jq -Rs '{text: .}'
  else
    printf '%s' "$1" | python3 -c 'import json,sys; print(json.dumps({"text": sys.stdin.read()}), end="")'
  fi
}

echo "mqtt-to-mattermost: listening on monTessie (localhost), JSON via $JSON_ENCODER" >&2

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
