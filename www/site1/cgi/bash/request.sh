#!/usr/bin/env bash

BODY="$(cat)"
BODY="${BODY#request=}"
printf '%s\r\n\r\n' "$BODY" | nc localhost 8080