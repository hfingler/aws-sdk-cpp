all: awssdk s3cpp

awssdk:
	mkdir -p sdkbuild && cd sdkbuild && cmake -D BUILD_ONLY="s3;transfer" -DCMAKE_INSTALL_PREFIX=${PREFIX} -DBUILD_SHARED_LIBS=OFF -DENABLE_TESTING=OFF ../aws-cpp-sdk && make install
	@echo "installing to ${PREFIX}"

s3cpp: s3cppclean
	mkdir s3cpp-build && cd s3cpp-build && cmake -DCMAKE_INSTALL_PREFIX=${PREFIX} ../s3cpp-lib && make && make install

s3cppclean:
	rm -rf s3cpp-build

clean: s3cppclean
	rm -rf sdkbuild
