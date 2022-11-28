#!/bin/bash
#
if [ -z $1 ]
then
	echo "Invalid arguments."
	exit 1
fi
echo "Checking git..."

script_dir=$(dirname -- "$0");
echo "Script directory is ${script_dir}"
echo "Destination directory is $1"

if [ ! -d $1 ]
then
	echo "Destination directory does not exist. Create."
	mkdir $1	
fi

header_file_in="${script_dir}/git_version.h"
source_file_in="${script_dir}/git_version.cpp.in"

header_file_out="$1/git_version.h"
source_file_out="$1/git_version.cpp"

cp ${header_file_in} ${header_file_out}

git_describe=$(git describe --tags --long --always)
git_diff=$(git diff --quiet --exit-code || echo +)
git_hash=$(git log -1 --format=%h)
git_branch=$(git rev-parse --abbrev-ref HEAD)
git_commit_num=$(git rev-list HEAD --count)

sed "s|@GIT_DESCRIBE@|${git_describe}|g;s|@GIT_DIFF@|${git_diff}|g;s|@GIT_HASH@|${git_hash}|g;s|@GIT_BRANCH@|${git_branch}|g;s|@GIT_COMMIT_NUM@|${git_commit_num}|g" ${source_file_in} > ${source_file_out}

echo "Done."
