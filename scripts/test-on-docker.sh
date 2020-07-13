#!/bin/bash

set -eux

docker build -t tym .
docker run tym
