#!/bin/sh

redis-cli SET foo bar px 100
redis-cli GET foo
sleep 0.2 && redis-cli GET foo