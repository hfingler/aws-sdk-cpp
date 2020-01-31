all: awssdk s3cpp

awssdk:
	mkdir -p sdkbuild && mkdir -p lib && cd sdkbuild && cmake -DBUILD_ONLY="s3;transfer" -DCMAKE_INSTALL_PREFIX=/usr/local -DENABLE_TESTING=OFF ../aws-cpp-sdk && make install && cd ..

s3cpp: s3cppclean
	mkdir s3cpp-build && cd s3cpp-build && cmake -DCMAKE_INSTALL_PREFIX=/usr/local -DCMAKE_PREFIX_PATH=/aws-sdk-cpp/sdkbuild ../s3cpp-lib && make && sudo make install && cd ..

s3cppclean:
	rm -rf s3cpp-build

clean: s3cppclean
	rm -rf sdkbuild
