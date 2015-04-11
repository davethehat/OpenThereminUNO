#!/bin/bash

os=`uname`

rc=${HOME}/.bashrc
aliases=${HOME}/.bash_aliases

echo "export OPEN_THEREMIN_DEV_HOME=$PWD" >> ${rc}
echo "alias ot='source $PWD/tools/dev-aliases.sh'" >> ${aliases}
