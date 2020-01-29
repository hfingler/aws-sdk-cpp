#include <aws/core/Aws.h>
#include <aws/core/client/ClientConfiguration.h>
#include <aws/core/http/Scheme.h>
#include <aws/core/utils/threading/Executor.h>
#include <aws/s3/S3Client.h>
#include <aws/transfer/TransferManager.h>
#include <boost/asio/streambuf.hpp>
#include <iostream>

using namespace std;
using namespace Aws::S3;
using namespace Aws::S3::Model;
using namespace Aws::Client;
using namespace Aws::Http;
using namespace Aws::Transfer;
using namespace Aws::Utils;

static const char* const ALLOCATION_TAG = "S3_MULTIPART_TEST";

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

int main(int /*argc*/, char** /*argv*/)
{
    Aws::SDKOptions options;
    Aws::InitAPI(options);

    Aws::String bucket = "hfn-usetl-output";

    // Set up S3 client
    ClientConfiguration config;
    config.region = "us-east-1";
    config.connectTimeoutMs = 3000;
    config.requestTimeoutMs = 60000;
    config.executor = Aws::MakeShared<Aws::Utils::Threading::PooledThreadExecutor>(ALLOCATION_TAG, 2);
    auto s3Client = Aws::MakeShared<S3Client>(ALLOCATION_TAG, config);

    auto sdk_client_executor = Aws::MakeShared<Aws::Utils::Threading::PooledThreadExecutor>(ALLOCATION_TAG, 4);
    TransferManagerConfiguration transferConfig(sdk_client_executor.get());
    transferConfig.s3Client = s3Client;

    //create TransferManager
    static std::shared_ptr<Aws::Transfer::TransferManager> transferManager = Aws::Transfer::TransferManager::Create(transferConfig);

    boost::asio::streambuf b;
    //std::ostream os(&b);

    auto creationFunction = [&](){ return Aws::New< StreamBuf >( "DownloadTag", &b); };

    std::shared_ptr<Aws::Transfer::TransferHandle> dhandle = 
            transferManager->DownloadFile("hfn-usetl-testing", "test.txt", creationFunction);

    dhandle->WaitUntilFinished();

    std::cout << dhandle->GetBytesTotalSize() << std::endl;


    char buf[20];
    for (int i = 0 ; i < 20 ; i++) {
        buf[i] = i;
    }

    //auto sstream = std::make_shared<std::stringstream>();
    //sstream->write(buf, 20);

    auto data = Aws::MakeShared<Aws::StringStream>("PutObjectInputStream", std::stringstream::in | std::stringstream::out | std::stringstream::binary);
    data->write(reinterpret_cast<char*>(buf), 20);

    std::shared_ptr<Aws::Transfer::TransferHandle> uhandle = 
        transferManager->UploadFile(data, "hfn-usetl-testing", "test2.txt", "application/octet-stream", Aws::Map<Aws::String, Aws::String>());

    uhandle->WaitUntilFinished();

/*
    transferConfig.uploadProgressCallback =
        [](const Aws::Transfer::TransferManager*, const std::shared_ptr<const Aws::Transfer::TransferHandle>&transferHandle)
    { std::cout << "Upload Progress: " << transferHandle->GetBytesTransferred() << " of " << transferHandle->GetBytesTotalSize() << " bytes\n"; };

    
    Aws::String fileToUpload = "MyFile.data";
    std::shared_ptr<Aws::Transfer::TransferHandle> transferHandle = 
            transferManager->UploadFile(fileToUpload.c_str(), bucket.c_str(), fileToUpload.c_str(),
                                        "multipart/form-data", Aws::Map<Aws::String, Aws::String>());

    size_t retries = 0;
    transferHandle->WaitUntilFinished();
    while (transferHandle->GetStatus() == Aws::Transfer::TransferStatus::FAILED && retries++ < 5)
    {
        transferManager->RetryUpload(fileToUpload.c_str(), transferHandle);
        transferHandle->WaitUntilFinished();
    }
    transferHandle->SetIsMultipart(true);

    Aws::Transfer::PartStateMap completedParts = transferHandle->GetCompletedParts();
    string eTagList;

    for (const auto &p : completedParts)
    {
        std::cout << "completedparts[" << p.first << "]" << p.second->GetETag() << '\n';
    }

    Aws::Transfer::TransferStatus status = transferHandle->GetStatus();
    std::string res("Result:");
    res.append(std::to_string((int)status));

    std::cout << res.c_str() << std::endl;
*/
    Aws::ShutdownAPI(options);
    return 0;
}