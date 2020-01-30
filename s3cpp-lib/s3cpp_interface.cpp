#include <aws/core/Aws.h>
#include <aws/core/client/ClientConfiguration.h>
#include <aws/core/http/Scheme.h>
#include <aws/core/utils/threading/Executor.h>
#include <aws/s3/S3Client.h>
#include <aws/transfer/TransferManager.h>
#include "boost/asio/streambuf.hpp"
#include <iostream>

using namespace std;
using namespace Aws::S3;
using namespace Aws::S3::Model;
using namespace Aws::Client;
using namespace Aws::Http;
using namespace Aws::Transfer;
using namespace Aws::Utils;

extern "C" void  s3cpp_upload(char* buf, uint32_t len, char* bucket, char* key);
extern "C" char* s3cpp_download(char* bucket, char* key);
extern "C" void  hello_world(char* x);

class StreamBuf : public Aws::IOStream
{
    public:
        using Base = Aws::IOStream;
        // provide a customer controlled streambuf, so as to put all transfered data into this in memory buffer.
        StreamBuf(boost::asio::streambuf* buf)
            : Base(buf)
        {}

        virtual ~StreamBuf() = default;
};


void hello_world(char* x) {
    printf("Hello, World! %s\n", x);
}

/*
static bool inited = false;
static std::shared_ptr<Aws::Transfer::TransferManager> transferManager;

void init_s3c() {
    if (inited) 
        return;
    
    Aws::SDKOptions options;
    Aws::InitAPI(options);

    ClientConfiguration config;
    config.region = "us-east-1";
    config.connectTimeoutMs = 3000;
    config.requestTimeoutMs = 60000;
    config.executor = Aws::MakeShared<Aws::Utils::Threading::PooledThreadExecutor>("USETL", 2);
    auto s3Client = Aws::MakeShared<S3Client>("USETL", config);

    auto sdk_client_executor = Aws::MakeShared<Aws::Utils::Threading::PooledThreadExecutor>("USETL", 4);
    TransferManagerConfiguration transferConfig(sdk_client_executor.get());
    transferConfig.s3Client = s3Client;

    transferConfig.uploadProgressCallback =
        [](const Aws::Transfer::TransferManager*, const std::shared_ptr<const Aws::Transfer::TransferHandle>&transferHandle)
    { std::cout << "Upload Progress: " << transferHandle->GetBytesTransferred() << " of " << transferHandle->GetBytesTotalSize() << " bytes\n"; };

    transferManager = Aws::Transfer::TransferManager::Create(transferConfig);
}
*/

void s3cpp_upload(char* buf, uint32_t len, char* bucket, char* key) {


    Aws::SDKOptions options;
    Aws::InitAPI(options);

    ClientConfiguration config;
    config.region = "us-east-1";
    config.connectTimeoutMs = 3000;
    config.requestTimeoutMs = 60000;
    config.executor = Aws::MakeShared<Aws::Utils::Threading::PooledThreadExecutor>("USETL", 2);
    auto s3Client = Aws::MakeShared<S3Client>("USETL", config);

    auto sdk_client_executor = Aws::MakeShared<Aws::Utils::Threading::PooledThreadExecutor>("USETL", 4);
    TransferManagerConfiguration transferConfig(sdk_client_executor.get());
    transferConfig.s3Client = s3Client;

    transferConfig.uploadProgressCallback =
        [](const Aws::Transfer::TransferManager*, const std::shared_ptr<const Aws::Transfer::TransferHandle>&transferHandle)
    { std::cout << "Upload Progress: " << transferHandle->GetBytesTransferred() << " of " << transferHandle->GetBytesTotalSize() << " bytes\n"; };

    static std::shared_ptr<Aws::Transfer::TransferManager>transferManager = Aws::Transfer::TransferManager::Create(transferConfig);




    auto data = Aws::MakeShared<Aws::StringStream>("USETL", std::stringstream::in | std::stringstream::out | std::stringstream::binary);
    //this copy is avoidable but requires a lot of effort (reimplement a istream_basic)
    data->write(reinterpret_cast<char*>(buf), len);

    //find a way to set MIME type?
    std::shared_ptr<Aws::Transfer::TransferHandle> uhandle = 
        transferManager->UploadFile(data, bucket, key, "application/octet-stream", Aws::Map<Aws::String, Aws::String>());

    uhandle->WaitUntilFinished();

    uint32_t retries = 0;
    while (uhandle->GetStatus() == Aws::Transfer::TransferStatus::FAILED && retries++ < 5)
    {
        transferManager->RetryUpload(data, uhandle);
        uhandle->WaitUntilFinished();
    }

    Aws::Transfer::TransferStatus status = uhandle->GetStatus();
    std::string res("Result:");
    res.append(std::to_string((int)status));

    std::cout << res.c_str() << std::endl;
}

char* s3cpp_download(char* bucket, char* key)
{
    Aws::SDKOptions options;
    Aws::InitAPI(options);

    ClientConfiguration config;
    config.region = "us-east-1";
    config.connectTimeoutMs = 3000;
    config.requestTimeoutMs = 60000;
    config.executor = Aws::MakeShared<Aws::Utils::Threading::PooledThreadExecutor>("USETL", 2);
    auto s3Client = Aws::MakeShared<S3Client>("USETL", config);

    auto sdk_client_executor = Aws::MakeShared<Aws::Utils::Threading::PooledThreadExecutor>("USETL", 4);
    TransferManagerConfiguration transferConfig(sdk_client_executor.get());
    transferConfig.s3Client = s3Client;

    transferConfig.uploadProgressCallback =
        [](const Aws::Transfer::TransferManager*, const std::shared_ptr<const Aws::Transfer::TransferHandle>&transferHandle)
    { std::cout << "Upload Progress: " << transferHandle->GetBytesTransferred() << " of " << transferHandle->GetBytesTotalSize() << " bytes\n"; };

    static std::shared_ptr<Aws::Transfer::TransferManager>transferManager = Aws::Transfer::TransferManager::Create(transferConfig);




    boost::asio::streambuf b;
    auto creationFunction = [&](){ return Aws::New< StreamBuf >( "DownloadTag", &b); };

    std::shared_ptr<Aws::Transfer::TransferHandle> dhandle = 
            transferManager->DownloadFile(bucket, key, creationFunction);

    dhandle->WaitUntilFinished();
    uint32_t retries = 0;
    while (dhandle->GetStatus() == Aws::Transfer::TransferStatus::FAILED && retries++ < 5)
    {
        transferManager->RetryDownload(dhandle);
        dhandle->WaitUntilFinished();
    }

    std::cout << "downloaded: " << dhandle->GetBytesTotalSize() << std::endl;
}