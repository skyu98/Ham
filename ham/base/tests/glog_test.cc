#include <glog/logging.h>

int main()
{
    FLAGS_logtostderr = true;

    google::InitGoogleLogging("Log test");
    LOG(INFO) << "INFO~~~~";
    LOG(WARNING) << "WARNING------";
    google::ShutdownGoogleLogging();
    return 0;
}