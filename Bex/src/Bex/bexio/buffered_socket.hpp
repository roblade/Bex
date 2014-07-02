#ifndef __BEX_IO_BUFFERED_SOCKET_HPP__
#define __BEX_IO_BUFFERED_SOCKET_HPP__

//////////////////////////////////////////////////////////////////////////
/// socket֮�Ϸ�װһ���շ�������

#include "bexio_fwd.hpp"
#include "allocator.hpp"

namespace Bex { namespace bexio
{
    template <typename Socket, typename Buffer, typename Allocator>
    class buffered_socket
        : boost::noncopyable
    {
        typedef typename Allocator::template rebind<char>::other alloc_t;
        typedef buffered_socket<Socket, Buffer, Allocator> this_type;

    public:
        typedef typename next_layer_t<Socket>::type next_layer_type;
        typedef typename lowest_layer_t<Socket>::type lowest_layer_type;

    public:
        // @rbsize : ���ջ�������С
        // @wbsize : ���ͻ�������С
        template <typename Arg /*= io_service*/>
        buffered_socket(Arg & a, std::size_t rbsize, std::size_t wbsize)
            : socket_(a)
            , read_storage_(alloc_t().allocate(rbsize))
            , read_buffer_(read_storage_, rbsize)
            , write_storage_(alloc_t().allocate(wbsize))
            , write_buffer_(write_storage_, wbsize)
        {}

        ~buffered_socket()
        {
            alloc_t().deallocate(read_storage_);
            alloc_t().deallocate(write_storage_);
        } 
        
        // ��ȡ��һ��
        next_layer_type& next_layer()
        {
            return socket_;
        }

        // ��ȡ��ײ�
        lowest_layer_type& lowest_layer()
        {
            return ::Bex::bexio::lowest_layer(socket_);
        }

        // nothing to speak...
        io_service& get_io_service()
        {
            return socket_.get_io_service();
        }

        // �ر�
        void close()
        {
            socket_.close();
        }

        // �ر�
        error_code close(error_code & ec)
        {
            return socket_.close(ec);
        }

        // ͬ��д��(����)����
        template <typename ConstBufferSequence>
        std::size_t write_some(const ConstBufferSequence& buffers)
        {
            return socket_.write_some(buffers);
        }

        // ͬ��д��(����)����
        template <typename ConstBufferSequence>
        std::size_t write_some(const ConstBufferSequence& buffers, error_code& ec)
        {
            return socket_.write_some(buffers, ec);
        }

        // �첽д��(����)����
        // @WriteHandler : void(boost::system::error_code, std::size_t))
        template <typename WriteHandler>
        bool async_write_some(BEX_IO_MOVE_ARG(WriteHandler) handler)
        {
            boost::array<const_buffer, 2> buffers;
            if (!write_buffer_.get_buffers(buffers))
                return false;

            socket_.async_write_some(buffers,
                boost::bind(&this_type::on_async_write<WriteHandler>, this
                    , placeholders::error, placeholders::bytes_transferred, handler));
            return true;
        }

        // ͬ����ȡ(����)����
        template <typename MutableBufferSequence>
        std::size_t read_some(const MutableBufferSequence& buffers)
        {
            return socket_.read_some(buffers);
        }

        // ͬ����ȡ(����)����
        template <typename MutableBufferSequence>
        std::size_t read_some(const MutableBufferSequence& buffers,
            boost::system::error_code& ec)
        {
            return socket_.read_some(buffers, ec);
        }

        // �첽��ȡ(����)����
        // @ReadHandler : void(boost::system::error_code, std::size_t)
        template <typename ReadHandler>
        bool async_read_some(BEX_IO_MOVE_ARG(ReadHandler) handler)
        {
            boost::array<mutable_buffer, 2> buffers;
            if (!read_buffer_.put_buffers(buffers))
                return false;

            socket_.async_read_some(buffers,
                boost::bind(&this_type::on_async_read<ReadHandler>, this
                    , placeholders::error, placeholders::bytes_transferred, handler));
            return true;
        }

        // ��ȡ���ջ������е����յ�������
        template <typename ConstBufferSequence>
        std::size_t get_buffers(ConstBufferSequence & buffers) const
        {
            return read_buffer_.get_buffers(buffers);
        }

        // ��ȡ�������
        void read_done(std::size_t size)
        {
            read_buffer_.gbump(size);
        }

        // ���ջ������ɶ����ݳ���
        std::size_t getable_read() const
        {
            return read_buffer_.gsize();
        }

        // ���ջ�������д���ݳ���
        std::size_t putable_read() const
        {
            return read_buffer_.psize();
        }

        // ��ȡ���ͻ������еĿ�д���ݶ�
        template <typename MutableBufferSequence>
        std::size_t put_buffers(MutableBufferSequence & buffers) const
        {
            return write_buffer_.put_buffers(buffers);
        }

        // д���������
        void write_done(std::size_t size)
        {
            write_buffer_.pbump(size);
        }

        // ���ͻ������ɶ����ݳ���
        std::size_t getable_write() const
        {
            return write_buffer_.gsize();
        }

        // ���ͻ�������д���ݳ���
        std::size_t putable_write() const
        {
            return write_buffer_.psize();
        }

        // ��Ҫ���͵�����д�뷢�ͻ�������
        std::size_t sputn(char const* data, std::size_t size)
        {
            return write_buffer_.sputn(data, size);
        }

        template <typename ConstBufferSequence>
        std::size_t sputn(ConstBufferSequence const& buffers)
        {
            return write_buffer_.sputn_to_buffers(buffers);
        }

        // �����ջ��������ѽ��յ�����copy��MutableBufferSequence��
        std::size_t sgetn(char const* data, std::size_t size)
        {
            return read_buffer_.sgetn(data, size);
        }

        template <typename MutableBufferSequence>
        std::size_t sgetn(MutableBufferSequence & buffers)
        {
            return read_buffer_.sgetn_to_buffers(buffers);
        }

    private:
        template <typename WriteHandler>
        void on_async_write(error_code ec, std::size_t bytes_transferred,
            BEX_IO_MOVE_ARG(WriteHandler) handler)
        {
            if (!ec)
                write_buffer_.gbump(bytes_transferred);
            handler(ec, bytes_transferred);
        }

        template <typename ReadHandler>
        void on_async_read(error_code ec, std::size_t bytes_transferred,
            BEX_IO_MOVE_ARG(ReadHandler) handler)
        {
            if (!ec)
                read_buffer_.pbump(bytes_transferred);
            handler(ec, bytes_transferred);
        }

    private:
        Socket socket_;
        Buffer read_buffer_;
        Buffer write_buffer_;
        char * read_storage_;
        char * write_storage_;
    };
    
} //namespace bexio
} //namespace Bex

#endif //__BEX_IO_BUFFERED_SOCKET_HPP__