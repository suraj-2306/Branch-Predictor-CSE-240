#!/usr/bin/env bash

set -ex

git remote add upstream $(cat .starter_repo) || true                  # You need to do this once each time you checkout a new lab. It will fail 
                                                                      # harmlessly if you run it more than once.
cp  -r src src_backup                                                 # backup yout work.
git commit -am "My progress so far." || true                          # commit your work.
# git pull --rebase upstream main --allow-unrelated-histories -X theirs # pull the updates
git reset --hard upstream/main
git push -f                                                           # push new base
