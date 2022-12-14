#!/bin/bash
echo "Pulling from submodule repositories..."
cd cmask
echo "Pulling from `pwd`..."
git checkout master
git pull
git branch
cd ..
cd libmusicxml
echo "Pulling from `pwd`..."
git checkout master
git pull
git branch
cd ..
cd portsmf
echo "Pulling from `pwd`..."
git checkout main
git pull
git branch
cd ..
cd eigen
echo "Pulling from `pwd`..."
git checkout master
git pull
git branch
cd ..
echo "Back in `pwd`."
echo "Finished pulling from submodule repositories."
