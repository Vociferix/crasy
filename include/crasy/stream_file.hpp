#ifndef CRASY_STREAM_FILE_HPP
#define CRASY_STREAM_FILE_HPP

// clang-fomrat off
#include <crasy/config.hpp>
// clang-format on

#include <asio/stream_file.hpp>
#include <crasy/future.hpp>
#include <crasy/mutex.hpp>
#include <crasy/result.hpp>
#include <filesystem>
#include <span>

namespace crasy {

class CRASY_API stream_file {
  public:
    using flags = asio::stream_file::flags;
    static inline constexpr read_only = asio::stream_file::read_only;
    static inline constexpr write_only = asio::stream_file::write_only;
    static inline constexpr read_write = asio::stream_file::read_write;
    static inline constexpr append = asio::stream_file::append;
    static inline constexpr create = asio::stream_file::create;
    static inline constexpr exclusive = asio::stream_file::exclusive;
    static inline constexpr truncate = asio::stream_file::truncate;
    static inline constexpr sync_all_on_write =
        asio::stream_file::sync_all_on_write;

    using seek_basis = asio::stream_file::seek_basis;
    static inline constexpr seek_set = asio::stream_file::seek_set;
    static inline constexpr seek_cur = asio::stream_file::seek_cur;
    static inline constexpr seek_end = asio::stream_file::seek_end;

    stream_file();
    stream_file(const stream_file&) = delete;
    stream_file(stream_file&&) = default;
    ~stream_file();
    stream_file& operator=(const stream_file&) = delete;
    stream_file& operator=(stream_file&&) = default;

    future<result<void>> open(const std::filesystem::path& path,
                              flags open_flags);

    future<result<void>> close();

    bool is_open() const;
    operator bool() const;

    future<result<std::size_t>> read_some(std::span<char> buffer);
    future<result<std::size_t>> read_some(std::span<unsigned char> buffer);
    future<result<std::size_t>> read_some(std::span<std::byte> buffer);

    future<result<std::size_t>> read(std::span<char> buffer);
    future<result<std::size_t>> read(std::span<unsigned char> buffer);
    future<result<std::size_t>> read(std::span<std::byte> buffer);

    future<result<std::size_t>> write_some(std::span<const char> buffer);
    future<result<std::size_t>> write_some(
        std::span<const unsigned char> buffer);
    future<result<std::size_t>> write_some(std::span<const std::byte> buffer);

    future<result<std::size_t>> write(std::span<const char> buffer);
    future<result<std::size_t>> write(std::span<const unsigned char> buffer);
    future<result<std::size_t>> write(std::span<const std::byte> buffer);

    future<result<void>> resize(uint64_t new_size);
    future<result<uint64_t>> size() const;

    future<result<uint64_t>> seek(int64_t offset, seek_basis whence);

    future<result<void>> sync_all();
    future<result<void>> sync_data();

  private:
    asio::stream_file file_;
    mutex read_mtx_;
    mutex write_mtx_;
};

} // namespace crasy

#endif
