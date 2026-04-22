#!/bin/bash

# remove the build directory if it's already there.
rm -rf build/ || true

# recreate it empty
mkdir -p build

# build the source in proc/
pushd ./proc/ || {
  echo -e 'Could not find the proc/ directory, did you delete it by mistake?' && exit 99 ;
}

# compile everything in proc/
make || {
  echo -e 'Build example source failed, recheck your cloned repository or contact your instructor' && exit 99 ;
}

# go out of of proc/
popd || {
  echo -e 'Something went wrong when switching directories, contact your instructor!' && exit 99 ;
}

# move into the build directory
pushd ./build/  || {
  echo -e 'Could not move into the build directory, contact your instructor!' && exit 99 ;
}

# generate the cmake files
cmake -DBUILD_SOLUTIONS=OFF -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DBUILD_BINUTILS=OFF --fresh .. || {
  echo -e 'Automatic cmake failed, try a manual cmake in the build directory or contact your instructor' && exit 99 ;
}

# link the proc/ files and the csse332.txt file
ln -s ../proc/*.rf .
ln -s ../proc/csse332.txt .

# compile everything
make all || {
  echo -e 'Final compilation step failed, this is likely a bug in your code, read the compilation errors and fix them!' && exit 99 ;
}

popd || true
echo -e 'All done and good to go!'

