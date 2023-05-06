# esp32-idf-studying

//================================================================================
//          UART EVEN
khi nhận dữ liệu từ phần cứng thì nó sẽ lưu vào BUFFER -> chuyển vào xQueueReciver;

+ check xem trong Queue có data không?
+ hàm malloc: tạo ra một phân vùng bộ đệm có bộ nhớ bằng BUF_SIZE
+ hàm bzero(str,int): clear phân vùng bộ nhớ đệm về NULL
+ hàm free(): giải phóng bộ nhớ

//cơ chế hoạt động của ring buffer: là gồm có 2 con trỏ Write và Read
    -   mỗi khi nhận lệnh ghi thì con trỏ pWrite sẽ ghi data vào bộ đệm sau đó sẽ tăng lên 1 đơn vị
    - mỗi khi  nhận lệnh đọc thi con trỏ pRead sẽ tăng lên 1 đơn vị và đọc giá trị từ bộ đệm ra. 
    - khi con trỏ tới cuối mảng thì nó sẽ cuộn lại vị trí đầu điên.
    - Nếu pRead == pWrite, chứng tỏ bộ đệm đang trống và k có gì để ghi
    - Nếu pWrite+1 == pRead, chứng tỏ bộ đẹm đang đầy và không thể ghi thêm
// thuật toán chương trình even

+ 1: Khởi tạo even, buffer_size và bộ nhớ đệm để lưu giá trị nhận (malloc), 
+ 2: kiểm tra có dũ liệu trong Queue không sau đó tới bước 3 (xQueueReceive)
+ 3: kiểm tra even:
    + + UART_DATA: uart nhận data
    + + UART_FIFO_OVF: uart tràn 
    + + UART_BUFFER_FULL: đệm buffer tràn
    + + UART_BREAK: ngắt uart
    + + UART_PARITY_ERR: lỗi bit kiểm tra chẵn lẻ
    + + UART_PATTERN_DET: uart pattern detected

//==============================================
 LƯU Ý:
    + + + KHI ĐỔI TÊN CÁI MAIN.C THÌ PHẢI ĐỔI CẢ TRONG FILE CMAKELIST