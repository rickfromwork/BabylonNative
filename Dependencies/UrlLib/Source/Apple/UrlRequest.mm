#include <UrlLib/UrlLib.h>
#include <arcana/threading/task.h>
#include <arcana/threading/task_schedulers.h>

#import <Foundation/Foundation.h>

namespace UrlLib
{
    class UrlRequest::Impl
    {
    public:
        ~Impl()
        {
            Abort();
            if (m_responseBuffer)
            {
                [m_responseBuffer release];
            }
        }

        void Abort()
        {
            m_cancellationSource.cancel();
        }

        void Open(UrlMethod method, std::string url)
        {
            m_method = method;
            m_url = std::move(url);
        }

        UrlResponseType ResponseType() const
        {
            return m_responseType;
        }

        void ResponseType(UrlResponseType value)
        {
            m_responseType = value;
        }

        arcana::task<void, std::exception_ptr> SendAsync()
        {
            // encode URL so characters like space are replaced by %20
            NSString* urlString = [[NSString stringWithUTF8String:m_url.data()] stringByAddingPercentEncodingWithAllowedCharacters:NSCharacterSet.URLQueryAllowedCharacterSet];
            NSURL* url{[NSURL URLWithString:urlString]};
            NSString* scheme{url.scheme};
            if ([scheme isEqual:@"app"])
            {
                NSString* path{[[NSBundle mainBundle] pathForResource:url.path ofType:nil]};
                if (path == nil)
                {
                    // Complete the task, but retain the default status code of 0 to indicate a client side error.
                    return arcana::task_from_result<std::exception_ptr>();
                }
                url = [NSURL fileURLWithPath:path];
            }

            NSURLSession* session{[NSURLSession sharedSession]};
            NSURLRequest* request{[NSURLRequest requestWithURL:url]};
            if (url == nil)
            {
                // Complete the task, but retain the default status code of 0 to indicate a client side error.
                return arcana::task_from_result<std::exception_ptr>();
            }

            __block arcana::task_completion_source<void, std::exception_ptr> taskCompletionSource{};

            id completionHandler{^(NSData* data, NSURLResponse* response, NSError* error)
            {
                if (error != nil)
                {
                    // Complete the task, but retain the default status code of 0 to indicate a client side error.
                    // TODO: Consider logging or otherwise exposing the error message in some way via: [[error localizedDescription] UTF8String]
                    taskCompletionSource.complete();
                    return;
                }
                
                if ([response class] == [NSHTTPURLResponse class])
                {
                    NSHTTPURLResponse* httpResponse{(NSHTTPURLResponse*)response};
                    m_statusCode = static_cast<UrlStatusCode>(httpResponse.statusCode);
                }
                else
                {
                    m_statusCode = UrlStatusCode::Ok;
                }

                if (data != nil)
                {
                    switch (m_responseType)
                    {
                        case UrlResponseType::String:
                        {
                            m_responseString = std::string{static_cast<const char*>(data.bytes), data.length};
                            break;
                        }
                        case UrlResponseType::Buffer:
                        {
                            [data retain];
                            m_responseBuffer = data;
                            break;
                        }
                        default:
                        {
                            taskCompletionSource.complete(arcana::make_unexpected(std::make_exception_ptr(std::runtime_error{"Invalid response type"})));
                        }
                    }
                }
                
                taskCompletionSource.complete();
            }};

            NSURLSessionDataTask* task{[session dataTaskWithRequest:request completionHandler:completionHandler]};
            [task resume];

            return taskCompletionSource.as_task();
        }

        UrlStatusCode StatusCode() const
        {
            return m_statusCode;
        }

        gsl::cstring_span<> ResponseUrl()
        {
            return m_responseUrl;
        }

        gsl::cstring_span<> ResponseString()
        {
            return m_responseString;
        }

        gsl::span<const std::byte> ResponseBuffer() const
        {
            if (m_responseBuffer)
            {
                return {reinterpret_cast<const std::byte*>(m_responseBuffer.bytes), static_cast<long>(m_responseBuffer.length)};
            }

            return {};
        }

    private:
        arcana::cancellation_source m_cancellationSource{};
        UrlResponseType m_responseType{UrlResponseType::String};
        UrlMethod m_method{UrlMethod::Get};
        std::string m_url{};
        UrlStatusCode m_statusCode{UrlStatusCode::None};
        std::string m_responseUrl{};
        std::string m_responseString{};
        NSData* m_responseBuffer{};
    };
}

#include <Shared/UrlRequest.h>
