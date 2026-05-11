#!/usr/bin/env bash
# Subscribe to Tessie monitor topic; forward alarm-like lines to Mattermost incoming webhook.
# Requires: MATTERMOST_WEBHOOK_URL (e.g. from systemd EnvironmentFile=), mosquitto_sub, curl, jq
# Logs to stderr -> visible in: journalctl -u mqtt-to-mattermost -f

set -uo pipefail

if [[ -z "${MATTERMOST_WEBHOOK_URL:-}" ]]; then
  echo "mqtt-to-mattermost: MATTERMOST_WEBHOOK_URL is unset" >&2
  exit 1
fi
for cmd in mosquitto_sub curl jq; do
  if ! command -v "$cmd" >/dev/null 2>&1; then
    echo "mqtt-to-mattermost: missing command: $cmd" >&2
    exit 1
  fi
done

echo "mqtt-to-mattermost: listening on monTessie (localhost), forwarding lines matching ==ALARM== or ==ERROR==" >&2

mosquitto_sub -h localhost -t monTessie -N | while IFS= read -r line; do
  case "$line" in
    *"==ALARM=="*|*"==ERROR=="*)
      echo "mqtt-to-mattermost: match, forwarding (${#line} chars)" >&2
      payload=$(printf '%s' "$line" | jq -Rs '{text: .}')
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
