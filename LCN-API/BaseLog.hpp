#include "CommonHeader.hpp"

class BaseLog
{
protected:
    virtual void Log(const std::string& message) = 0;
};

class WS2Log : public BaseLog
{
public:
    void Log(const std::string& message) override
    {
        std::cout << "[WS2]> " << message << '\n';
    }
};

class NSLog : BaseLog
{
public:
    void Log(const std::string& message) override
    {
        if (!message.empty())
        {
            std::cout << "[S]> " << message << '\n';
        }
    }

    void ClientMessage(const std::string& clientIdentity, const std::string& clientMessage) const
    {
        std::cout << "[" << clientIdentity.c_str() << "]> " << clientMessage.c_str() << '\n';
    }

    class NSLogStream {
    public:
        NSLogStream(NSLog& nslog) : nslog_(nslog) {}

        NSLogStream(const NSLogStream&) = delete;
        NSLogStream& operator=(const NSLogStream&) = delete;

        NSLogStream(NSLogStream&& other) noexcept
            : nslog_(other.nslog_), stream_(std::move(other.stream_)) {
        }

        NSLogStream& operator=(NSLogStream&& other) noexcept {
            if (this != &other) {
                nslog_ = other.nslog_;
                stream_.str(other.stream_.str());
            }
            return *this;
        }

        NSLogStream& operator<<(const char* value) {
            stream_ << value;
            return *this;
        }

        NSLogStream& operator<<(char* value) {
            stream_ << value;
            return *this;
        }

        NSLogStream& operator<<(char value) {
            stream_ << value;
            return *this;
        }

        NSLogStream& operator<<(unsigned short value) {
            stream_ << value;
            return *this;
        }

        template<std::size_t N>
        NSLogStream& operator<<(const std::array<char, N>& arr) {
            stream_ << arr.data();
            return *this;
        }

        ~NSLogStream()
        {
            nslog_.Log(stream_.str());
        }

    private:
        NSLog& nslog_;
        std::ostringstream stream_;
    };

    NSLogStream CreateNSLogStream()
    {
        return NSLogStream(*this);
    }
};

class NCLog : BaseLog
{
public:
    void Log(const std::string& message) override
    {
        std::cout << "[C]> " << message << '\n';
    }

    void LocalLog(const std::string& message) const
    {
        std::cout << message << '\n';
    }

    void PromptIdentty() const
    {
        std::cout << "[Name]: ";
    }

    void PromptMessage() const
    {
        std::cout << "> ";
    }

    class NCLogStream {
    public:
        NCLogStream(NCLog& nclog) : nclog_(nclog) {}

        NCLogStream(const NCLogStream&) = delete;
        NCLogStream& operator=(const NCLogStream&) = delete;

        NCLogStream(NCLogStream&& other) noexcept
            : nclog_(other.nclog_), stream_(std::move(other.stream_)) {
        }

        NCLogStream& operator=(NCLogStream&& other) noexcept {
            if (this != &other) {
                nclog_ = other.nclog_;
                stream_.str(other.stream_.str());
            }
            return *this;
        }

        NCLogStream& operator<<(const char* value) {
            stream_ << value;
            return *this;
        }

        NCLogStream& operator<<(char* value) {
            stream_ << value;
            return *this;
        }

        NCLogStream& operator<<(char value) {
            stream_ << value;
            return *this;
        }

        NCLogStream& operator<<(unsigned short value) {
            stream_ << value;
            return *this;
        }

        template<std::size_t N>
        NCLogStream& operator<<(const std::array<char, N>& arr) {
            stream_ << arr.data();
            return *this;
        }

        ~NCLogStream()
        {
            nclog_.LocalLog(stream_.str());
        }

    private:
        NCLog& nclog_;
        std::ostringstream stream_;
    };

    NCLogStream CreateNCLogStream()
    {
        return NCLogStream(*this);
    }
};