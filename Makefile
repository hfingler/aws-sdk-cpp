all: awssdk s3cpp

awssdk:
	mkdir -p sdkbuild && cd sdkbuild && cmake -DBUILD_ONLY="s3;transfer" -DCMAKE_INSTALL_PREFIX=/usr/local -DBUILD_SHARED_LIBS=OFF -DENABLE_TESTING=OFF ../aws-cpp-sdk && make install && cd ..

s3cpp: s3cppclean
	mkdir s3cpp-build && cd s3cpp-build && cmake -DBUILD_SHARED_LIBS=OFF -DCMAKE_INSTALL_PREFIX=/usr/local -DCMAKE_PREFIX_PATH=/aws-sdk-cpp/sdkbuild ../s3cpp-lib && make && make install && cd ..

s3cppclean:
	rm -rf s3cpp-build

clean: s3cppclean
	rm -rf sdkbuild
