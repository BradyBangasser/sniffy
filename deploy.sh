#!/usr/bin/env sh

sip="10.49.136.143"

python3 go-builder.py
scp -r main.py api ds:
