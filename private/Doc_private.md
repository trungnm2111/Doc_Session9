# 2/ System call
System Call Interface (SCI) là “cửa” duy nhất được kiểm soát, nơi chương trình ở user space yêu cầu kernel làm giúp những việc nó không được tự làm: đọc/ghi file, tạo tiến trình, cấp phát bộ nhớ, mở socket... Mỗi việc như vậy là một system call (read, write, open, fork, mmap...), được đánh số trong một bảng (sys_call_table).

Nó giải quyết 3 vấn đề chính:

Bảo vệ hệ thống. CPU chia hai chế độ: user mode (bị hạn chế) và kernel mode (toàn quyền với phần cứng). Nếu ứng dụng được chạm thẳng vào ổ đĩa hay bộ nhớ của tiến trình khác, một chương trình lỗi hoặc độc hại có thể làm sập cả máy. SCI buộc mọi yêu cầu đi qua một điểm duy nhất, nơi kernel kiểm tra quyền và tham số trước khi làm.
Trừu tượng hóa phần cứng. Chương trình chỉ gọi write(fd, buf, n) mà không cần biết dữ liệu đi tới ổ SSD, terminal hay card mạng. Kernel lo phần driver.
Giao diện ổn định. Kernel bên trong thay đổi liên tục, nhưng số hiệu và ý nghĩa syscall gần như không đổi (“we don’t break userspace”), nên chương trình biên dịch từ nhiều năm trước vẫn chạy được.

Luồng hoạt động (x86-64): chương trình gọi hàm read() của glibc → glibc đặt số syscall vào thanh ghi rax, tham số vào rdi, rsi, rdx... → lệnh syscall chuyển CPU sang kernel mode → kernel tra bảng, chạy sys_read → kết quả trả về rax, CPU quay lại user mode.

## 2.1 Bản chất 

Nói gọn trong một câu: system call là cách duy nhất để một chương trình không có quyền được “nhờ” một bên có quyền làm việc giúp nó, qua một cánh cửa mà bên có quyền tự chọn và tự canh.

Phần tinh hoa nằm ở chỗ có 3 ý ghép lại với nhau:

Quyền lực do phần cứng ép buộc, không phải do thỏa thuận. CPU thật sự từ chối thực thi lệnh đặc quyền khi đang ở user mode. Nên ứng dụng không thể “lén” làm, kể cả khi nó muốn.
Chỉ có một lối lên, và lối đó do kernel đặt sẵn. Lệnh syscall không cho phép nhảy tới một địa chỉ tùy ý trong kernel. CPU luôn nhảy tới đúng một điểm vào mà kernel đã đăng ký từ lúc khởi động. Ứng dụng chỉ chọn được muốn gì (số syscall + tham số), còn có cho hay không và làm thế nào thì hoàn toàn do kernel quyết.
Yêu cầu là dữ liệu, không phải mã lệnh. Ứng dụng chỉ gửi những con số. Kernel coi chúng là đầu vào không đáng tin, kiểm tra hết rồi mới chạy code của chính nó.

Hình dung giống quầy giao dịch ngân hàng: bạn không được vào kho tiền, nhưng có thể đưa phiếu yêu cầu qua ô cửa. Nhân viên kiểm tra giấy tờ rồi tự vào kho lấy tiền cho bạn. Ô cửa đó chính là system call interface.

Mọi lợi ích khác (bảo vệ, trừu tượng phần cứng, giao diện ổn định) đều là hệ quả của ý tưởng “ranh giới đặc quyền + một cửa vào có kiểm soát” này. Đó cũng là lý do ioctl, procfs hay Netlink rốt cuộc vẫn phải đi qua syscall: chúng chỉ là những kiểu “phiếu yêu cầu” khác nhau đưa qua cùng một ô cửa.


1. “Đặt số vào thanh ghi rồi gọi lệnh syscall để chuyển quyền cho kernel”: đúng.
Có một chỗ cần chỉnh: không phải “chương trình nhường cho kernel chạy” như hai tiến trình khác nhau. CPU vẫn đang chạy chính tiến trình của bạn, chỉ là đổi sang kernel mode và nhảy vào code của kernel. Kernel làm việc thay mặt tiến trình đó, xong thì lệnh sysret đưa CPU về user mode, chạy tiếp ngay sau lệnh syscall.

2. “Bảng system call nằm sẵn trên thanh ghi à?”: không. Có hai thứ hay bị nhầm:
• Thanh ghi rax chỉ chứa số thứ tự (ví dụ 1 = write), chứ không chứa bảng.
• Bảng sys_call_table là một mảng con trỏ hàm nằm trong bộ nhớ kernel. Nó được tạo lúc biên dịch kernel từ file arch/x86/entry/syscalls/syscall_64.tbl. Phần tử số 1 trỏ tới sys_write, số 2 trỏ tới sys_open...
• Thanh ghi thật sự liên quan là MSR_LSTAR: lúc khởi động, kernel ghi vào đó địa chỉ cửa vào (entry_SYSCALL_64). Khi gặp lệnh syscall, CPU nhảy tới địa chỉ này. Code ở đó lấy rax làm chỉ số, tra bảng và gọi hàm tương ứng.
→ Phần cứng chỉ biết “cửa ở đâu”, còn kernel biết “số nào ứng với việc gì”.

3. “Có bao nhiêu số?”
Trên x86-64 với kernel hiện tại là khoảng 470 số, đánh từ 0, có một đoạn bỏ trống ở giữa. Mỗi kiến trúc đánh số riêng (ARM64 khác x86-64), và số chỉ được thêm chứ không bao giờ đổi nghĩa. Tự xem trên máy bạn: cat /usr/include/x86_64-linux-gnu/asm/unistd_64.h.

4. “Có gán chương trình của tôi vào một số để thành syscall được không?”
Được, nhưng không phải bằng một chương trình user. Code của syscall phải chạy trong kernel mode, mà nếu user tự đưa code vào đó được thì cả ranh giới bảo vệ ở trên mất ý nghĩa. Cách đúng là sửa và biên dịch lại kernel:



1. Character device interface











Bản chất
Character device là interface cho phép user-space giao tiếp với thiết bị/kernel driver thông qua file, thường nằm dưới /dev.

Ví dụ:
User application
       │
       │ open()
       │ read()
       │ write()
       │ ioctl()
       ▼
   /dev/device
       │
       ▼
Character driver
       │
       ▼
Hardware / Kernel subsystem

Dữ liệu được xử lý theo kiểu stream/byte sequence, tức có thể đọc/ghi một chuỗi byte.
Đặc điểm
- Có thể dùng open(), read(), write(), ioctl().
- User-space nhìn thiết bị gần giống như một file.
- Cho phép truyền dữ liệu giữa user-space và kernel-space.
- Có thể mở rộng nếu thiết kế receiver đủ tốt.
- Thường gắn với device driver và hardware.
Ưu điểm
Khá linh hoạt trong việc truyền dữ liệu.

Ví dụ:
write(fd, buffer, size);

Có thể truyền một lượng dữ liệu tùy thuộc vào interface.

Ngoài ra có thể kết hợp với ioctl() để thực hiện các operation/configuration đặc biệt.
Nhược điểm
Vấn đề lớn nhất là ý nghĩa của interface phụ thuộc vào từng driver.

Ví dụ:
/dev/deviceA → driver A
/dev/deviceB → driver B
/dev/deviceC → driver C

Mỗi driver có thể định nghĩa semantics khác nhau.

Tác giả cũng lo ngại việc dùng character/block device cho các subsystem không liên quan đến hardware sẽ dẫn đến nhiều interface tương tự nhưng khác nhau, gây duplication.
Khi nào phù hợp?
Rất phù hợp khi:
Kernel subsystem thực sự đại diện cho một device/hardware.

Ví dụ thiết bị serial, sensor, driver phần cứng...
2. Block device interface











Bản chất
Block device cũng được truy cập thông qua file/device node, nhưng khác character device ở chỗ dữ liệu được tổ chức theo block.

Ví dụ điển hình:
/dev/sda
/dev/nvme0n1

Đây là các block device đại diện cho storage.

Luồng cơ bản:
Application
     │
     ▼
Filesystem
     │
     ▼
Block layer
     │
     ▼
Block driver
     │
     ▼
SSD / HDD

Đặc điểm
- Dùng cho thiết bị lưu trữ.
- Dữ liệu được truy cập theo các block.
- Có thể đọc/ghi dữ liệu lớn.
- Được tích hợp sâu với filesystem và block layer của Linux.
Ưu điểm
Phù hợp với large data transfer, đặc biệt đối với storage.

Ví dụ đọc hàng MB/GB dữ liệu không cần biến mỗi byte thành một kernel interface riêng.
Nhược điểm
Tương tự character device:
Nó chủ yếu được thiết kế cho hardware/device, không phải interface tổng quát cho mọi kernel subsystem.

3. /proc interface











Bản chất
/proc là một virtual filesystem.

Nó không đại diện cho file thật trên disk.

Thay vào đó:
User-space
    │
    │ read/write
    ▼
  /proc/xxx
    │
    ▼
Kernel

Kernel tạo ra nội dung /proc dynamically.

Ví dụ:
/proc/cpuinfo
/proc/meminfo
/proc/<pid>/

Mục đích ban đầu
/proc chủ yếu được thiết kế để cung cấp:
thông tin về process và trạng thái của kernel

Ví dụ:
/proc/<pid>/status
/proc/<pid>/stat

Đặc điểm quan trọng
Thông tin thường ở dạng text human-readable.

Ví dụ:
MemTotal:       16384000 kB
MemFree:         2048000 kB

Con người đọc rất dễ.

Nhưng application muốn sử dụng thì phải:
"16384000 kB"
       ↓
parse
       ↓
integer
       ↓
internal representation

Ưu điểm
- Rất đơn giản.
- Dễ debug.
- Dễ quan sát bằng command line.
- Human-readable.
- Có thể sử dụng các thao tác file quen thuộc.
Nhược điểm
Đây là interface có khá nhiều hạn chế:

1. Không có event-based signaling tốt

Application phải chủ động kiểm tra.
Application
    │
    ├── read
    ├── read
    ├── read
    └── read

2. Khó extensible

Nếu format thay đổi:
version 1:
A B C

version 2:
A B C D

application cũ có thể không hiểu format mới.

3. Text không hiệu quả cho application

Phải parse và convert.

4. Data transfer bị giới hạn

Đoạn tài liệu bạn đưa nói rằng transfer bị giới hạn khoảng một memory page.

5. Ad-hoc

Mỗi interface /proc có thể được thiết kế cho một mục đích cụ thể, thiếu một format/interface thống nhất.
Khi nào nên dùng?
Rất phù hợp cho:
Quan sát/debug/thông tin trạng thái của process hoặc kernel.

Không lý tưởng cho một protocol cấu hình phức tạp giữa application và kernel.
4. Sysfs











Bản chất
Sysfs cũng là virtual filesystem, nhưng được thiết kế để biểu diễn:
- kernel objects
- devices
- device attributes
- relationships giữa các objects
Thông thường nằm ở:
/sys/

Ví dụ về mặt khái niệm:
/sys/
 └── devices/
      └── deviceA/
           ├── enable
           ├── status
           └── size

Ý tưởng quan trọng nhất
Sysfs áp dụng nguyên tắc:
One configuration parameter per file

Ví dụ:
/sys/device/enable
/sys/device/status
/sys/device/speed

thay vì nhét tất cả vào một file format phức tạp.
Ưu điểm
1. Có cấu trúc hơn /proc

Sysfs được tạo ra một phần để tránh việc dùng /proc cho các thông tin không liên quan đến process.

2. Có event-based signaling

Sysfs hỗ trợ event notification thông qua Netlink.

Điều này tốt hơn /proc.
Kernel
   │
   │ event
   ▼
Netlink
   │
   ▼
User application

3. Dễ quan sát

Vẫn sử dụng filesystem interface nên rất dễ debug bằng command line.
Nhược điểm
1. Text-based

Giống /proc, dữ liệu thường ở dạng text.

Application phải parse.

2. Giới hạn kích thước

Data transfer vẫn bị giới hạn khoảng một memory page.

3. ABI không hoàn toàn stable

Đoạn tài liệu nhấn mạnh:
Linux kernel không cung cấp một stable ABI.

Do đó không nên giả định mọi chi tiết của sysfs sẽ giữ nguyên giữa tất cả kernel releases.
Khi nào phù hợp?
Rất phù hợp cho:
Device attributes, device configuration đơn giản và kernel object information.

Ví dụ:
/sys/class/...
/sys/devices/...

Nhưng không lý tưởng cho:
cấu hình phức tạp hoặc truyền lượng dữ liệu lớn.

5. ioctl
ioctl hơi khác các interface trên vì nó là system-call-based mechanism thường được sử dụng thông qua device/file descriptor.

Ví dụ:
ioctl(fd, COMMAND, &config);

Luồng:
User-space
    │
    │ ioctl()
    ▼
File/device interface
    │
    ▼
Kernel subsystem

Đặc điểm
Mỗi operation có một command number:
COMMAND_A → làm A
COMMAND_B → làm B
COMMAND_C → làm C

và thường truyền pointer tới một data structure:
struct config {
    int width;
    int height;
};

Ưu điểm
- Có thể truyền cấu trúc dữ liệu tương đối phức tạp.
- Hiệu quả hơn việc parse text.
- Có thể thực hiện nhiều configuration operation.
- Rất phổ biến trong device driver.
Nhược điểm lớn
Fixed-layout structure.

Ví dụ version 1:
struct config {
    int a;
    int b;
};

Version 2 muốn thêm:
struct config {
    int a;
    int b;
    int c;
};

Binary layout thay đổi.

→ Có nguy cơ phá backward compatibility.

Do đó phải tạo:
IOCTL_V1
IOCTL_V2
IOCTL_V3
...

Dẫn đến nhiều interface dư thừa.

Ngoài ra còn vấn đề 32-bit/64-bit compatibility, đặc biệt với pointer và long.
6. BSD Socket interface
Đoạn bạn gửi bắt đầu chuyển sang loại này ở phần 2.2.

Ý tưởng là dùng BSD socket API để giao tiếp với kernel networking subsystem.

Ví dụ khái niệm:
User-space application
        │
        │ socket()
        │ send()/recv()
        ▼
   BSD Socket API
        │
        ▼
Kernel networking subsystem

Trong Linux networking, cách truyền thống là sử dụng:
datagram socket + ioctl

để cấu hình networking subsystem từ user-space.
Ưu điểm của socket approach
Socket vốn đã hỗ trợ:
- gửi/nhận dữ liệu
- message-based communication
- networking semantics
- nhiều loại address/family
Vì vậy nó có tiềm năng trở thành interface khá mạnh cho kernel subsystem.



So sánh toàn bộ
Interface	Mục đích chính	Format	Event	Extensible	Large data	Phù hợp nhất
Character device	Giao tiếp device	Binary/driver-defined	Tùy driver	Khá tốt nếu thiết kế tốt	Tốt	Hardware/device driver
Block device	Storage	Block/binary	Có cơ chế I/O	Tốt	Rất tốt	HDD/SSD/storage
/proc	Process/kernel information	Text	❌	❌	❌	Monitoring/debug
sysfs	Device/kernel objects	Text	✅	Khá hạn chế	❌	Device attributes/config đơn giản
ioctl	Configuration/control	Struct/binary	Không phải điểm mạnh	❌ nếu fixed struct	Khá	Device control/configuration
BSD socket	Message/data communication	Binary/message	✅/có thể	Tốt hơn	Tốt	Networking/kernel subsystems


Cách nhớ cực kỳ đơn giản
Bạn có thể nhớ 6 loại này bằng câu hỏi "Tôi muốn giao tiếp với kernel để làm gì?"
🟢 Muốn điều khiển hardware
→ Character device / Block device
/dev/...

🟡 Muốn xem thông tin process/kernel
→ /proc
/proc/...

🔵 Muốn xem/cấu hình thuộc tính của device/kernel object
→ sysfs
/sys/...

🟠 Muốn gửi command/configuration có cấu trúc
→ ioctl
ioctl(fd, CMD, &data);

🔴 Muốn communication kiểu message
→ BSD socket
socket()
send()
recv()

Và đây mới là vấn đề nghiên cứu chính của đoạn tài liệu
Tác giả không đơn thuần giới thiệu các interface này. Họ đang chứng minh rằng mỗi interface đều có những điểm yếu:
                 Kernel Interface

 Character/Block ─── tốt cho hardware
        │
        └── không phù hợp kernel subsystem tổng quát

 /proc ──────────── đơn giản
        │
        ├── text
        ├── không event
        ├── khó mở rộng
        └── data nhỏ

 sysfs ──────────── tốt hơn /proc
        │
        ├── có event
        ├── nhưng vẫn text
        ├── data nhỏ
        └── ABI không hoàn toàn stable

 ioctl ──────────── binary/structured
        │
        ├── hiệu quả
        ├── nhưng fixed layout
        ├── compatibility problem
        └── 32/64-bit problem

 BSD socket ─────── message communication
        │
        └── được dùng nhiều trong networking

Thông điệp cốt lõi: Linux có rất nhiều cách để user-space nói chuyện với kernel, nhưng không có interface truyền thống nào đồng thời giải quyết tốt cả 4 yêu cầu mà tác giả đặt ra:
portability + event signaling + extensibility + large data transfer

Đó chính là lý do phần tiếp theo của tài liệu sẽ có ý nghĩa: tác giả đang hướng tới một kernel interface/design mới hoặc tốt hơn, thay vì tiếp tục phụ thuộc hoàn toàn vào ioctl, /proc, sysfs hay device interface truyền thống.




Trong đó /dev là giao diện giúp ta có thể giao tiếp với các device driver  và phần cứng nó giúp ta hỗ trợ có thể truyền và gửi nhiều dữ liệu cũng như phương thức đa dạng linh hoạt, nhưng bên cạnh đó ta chỉ sử dụng được với các hardware driver còn những subsystem khác ta không thể dùng lý do nếu tạo ra hết sẽ rất rối và phức tạp, bên cạnh đó ta có proc giúp ta thuận tiện từ user để xem và chỉnh sửa các thuộc tính thống số dưới kernel như power, hay thông tin trạng thái các process  nhưng nó chỉ hỗ trợ cho dữ liệu người dùng tức terminal hạn chế khi ta sử dụng chương trình cần dịch lại chuyển đổi và sys là interface giúp ta quan sát và chỉnh sửa các thuộc tính kernel quản lý ra hướng người dùng đa dạng dễ dàng ngoài ra ta cũng có thể kết hợp với system call read , write để truy cập và chỉnh sửa thông tin 