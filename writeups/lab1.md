Lab 1 Writeup
=============

## byte_stream
写一个可读可写的队列，类似于生产者-消费者模型

一边写数据，另一边读数据
写的限制：管道满了，就不可写了
读的限制：管道空了，就不可读了

难度不大，对照测试用例进行调试即可

## stream_reassembler
发送端将报文拆成碎片发送给接收端，由于网络层是"尽力而为"的交付,因此接收端接收到的报文可能是乱序、重复等各种情况，因此接收端需要对其进行重组。除了对报文进行重组，接收端还需要做的事情就是“flow control”，即根据自己的remain_capacity，限制发送端发送的报文。

接收端的任务：1. reassemble 2. flow control

具体来说，capacity 要求重组好但未被应用层读取的数据(在管道中)和接收端缓冲的尚不能进行重组的报文总的数据量不能超过capacity

接收端收到的报文可以分为三类，如下所示，假设竖线左边是已经重组好的数据，竖线代表尚未被确认的sequence，假设每一个报文的sequence为index,长度为length。那么：

1. index <= sequence && index + length > sequence, 该报文的后半部分是有效内容，需要assemble
2. index <= sequence && index + length <= sequence,该报文的全部内容已经assemble过，需要丢弃(包括了index == sequence && length == 0)
3. index > sequence. 该报文需要缓存起来

``` cpp
packet can be 3 category
    -------|......
        i.....    1. tailor and then assemble
    i...       2. discard
            i... 3. buffer
```

由上可得，判断一个报文是否可以合并的关键条件是其index与当前sequence的大小，因此对于那些缓冲的报文，可以用一个优先队列存储起来。

在实际处理以上三类报文的时候，需要注意的是总的capacity的限制，以及存放重组好的报文的byte_stream的remaining_capacity的限制（可能存在其内已经充满了数据，写不下了）,还要注意如何判断eof

1. 处理第一类报文： 
    1. 注意判断管道的可写容量，当该报文处理完毕后
    2. 在优先队列中依次取队头报文尝试合并，直至遇见第一个无法合并的报文位置（优先队列性质）
    3. 更新数据（_end, _pool_size, _assembled_bytes）
2. 处理第二类报文：
    1. 丢弃并更新_end

3. 处理第三类报文：
    1. 判断该报文所有byte的index 是否大于当前能存储的最大sequence number, 大于的部分要丢弃

        *max_seq = read_first + capacity* 我们可以获取到当前未被读的第一个byte的seq no,
        根据capacity，可以确定当前最大的seq no, index超过这个的byte都应该被舍弃，否则可能出现死锁，
        即后面的byte占据了前面byte的位置，导致前面的byte无法被合并，每次接收到前面的bytes时，由于capacity的缘故都会被丢弃
    
    2. 进行第一步的裁剪之后，还需要进一步判断当前剩余容量能够存储多少bytes

    3. 更新数据（_end, _pool_size, _assembled_bytes）

注意：什么时候byte_stream不会在写入到数据了？ if no substrings are waiting to be assembled
    _end && !priority_queue.size()  ==> _output.end_input()