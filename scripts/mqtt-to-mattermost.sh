#!/bin/bash
mosquitto_sub -h localhost -t monTessie -N | while read -r line; do
  case "$line" in
    *"==ALARM=="*|*"==ERROR=="*)  # tighten to what you care about
      curl -sS -X POST -H 'Content-Type: application/json' \
        -d "{\"text\": $(echo "$line" | jq -Rs .)}" \
        "$MATTERMOST_WEBHOOK_URL"
      ;;
  esac
done