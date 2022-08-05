#!/bin/bash
echo "Pulling from submodule repositories..."
cd libmusicxml
echo "Pulling from `pwd`..."
git checkout master
git pull
git branch
cd ..
echo "Finished pulling from submodule repositories."
echo "Back in `pwd`."
git branch

