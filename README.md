# esp32-idf-studying

//================================================================================
// UART EVEN
khi nhận dữ liệu từ phần cứng thì nó sẽ lưu vào BUFFER -> chuyển vào xQueueReciver;

- check xem trong Queue có data không?
- hàm malloc: tạo ra một phân vùng bộ đệm có bộ nhớ bằng BUF_SIZE
- hàm bzero(str,int): clear phân vùng bộ nhớ đệm về NULL
- hàm free(): giải phóng bộ nhớ

//cơ chế hoạt động của ring buffer: là gồm có 2 con trỏ Write và Read - mỗi khi nhận lệnh ghi thì con trỏ pWrite sẽ ghi data vào bộ đệm sau đó sẽ tăng lên 1 đơn vị - mỗi khi nhận lệnh đọc thi con trỏ pRead sẽ tăng lên 1 đơn vị và đọc giá trị từ bộ đệm ra. - khi con trỏ tới cuối mảng thì nó sẽ cuộn lại vị trí đầu điên. - Nếu pRead == pWrite, chứng tỏ bộ đệm đang trống và k có gì để ghi - Nếu pWrite+1 == pRead, chứng tỏ bộ đẹm đang đầy và không thể ghi thêm
// thuật toán chương trình even

- 1: Khởi tạo even, buffer_size và bộ nhớ đệm để lưu giá trị nhận (malloc),
- 2: kiểm tra có dũ liệu trong Queue không sau đó tới bước 3 (xQueueReceive)
- 3: kiểm tra even:
  - - UART_DATA: uart nhận data
  - - UART_FIFO_OVF: uart tràn
  - - UART_BUFFER_FULL: đệm buffer tràn
  - - UART_BREAK: ngắt uart
  - - UART_PARITY_ERR: lỗi bit kiểm tra chẵn lẻ
  - - UART_PATTERN_DET: uart pattern detected

//==============================================
LƯU Ý:

- - - KHI ĐỔI TÊN CÁI MAIN.C THÌ PHẢI ĐỔI CẢ TRONG FILE CMAKELIST + + + Khi viết thư viện riêng cần thêm dòng
      set(EXTRA_COMPONENT_DIRS ${CMAKE_CURENT_LIS_DIR}/<folder>)

* - - sử dụng thư viện nhật kí: logging libary
      ESP_LOGI: ghi thông tin
      ESP_LOGE: ghi lỗi
      ESP_LOGW: ghi cảnh báo
      ESP_LOGD: gỡ lỗi
      ESP_LOGV:
      nhớ add vào file README

// debug sử dụng log libary và UART 1
// để lấy thời gian mS thì sử dụng hàm

//=================================================//

- - - - - - - - - SOFT TIMER + + + + + + + + + +

* bộ chia nhỏ nhất của SOFT Timer là 10ms
  \*\* Lưu ý: sử dụng timer của RTOS không phải timer cứng của ESP và bên dưới là các bước config 1 timmer

- khởi tạo Timer:
  TimerHandle_t xTimer[2];
- config timer:
  xTimer[0] = xTimerCreate("Timer Blink", // Ten Cua Timer
  pdMS_TO_TICKS(500), // Thoi gian tre ms doc them o trang chu cua rtos
  pdTRUE, // co autoReload khong
  (void \*)0, // ID cua timer
  vTimer_Callback); // ham calll back cua timer

- start timer:
  xTimerStart(xTimer[0], 0);
- hàm vTimer_Callback(): sử dụng để gọi hàm thực thi khi timer đầy;
  - - lưu ý trong hàm này:
      \*\*\* bước 1 : Kiểm tra lỗi configASSERT(xTimer);
      \*\*\* bước 2: Khiểm tra xem Timer nào đang ngắt rồi thực thi lệnh trong timer nó: pvTimerGetTimerID(xTimer);

//============================================//
EvenGroup : trường hợp 1 task thực thi nhiệm vụ là đọc nút bấm còn 1 task nhiệm vụ in data thì ra sử dụng evenGroup của RTOS;
//============================================//
UART COMMAND LINE INTERFACE
//=============================================//
để nhúng ảnh + file vào flash.(4M)
sử dụng lệnh:
target_add_binary_data(${CMAKE_PROJECT_NAME}.elf "html/index.html" TEXT)
//nếu gửi html "html/index.html": path
//thêm ở file Cmakelist,txt

file component: trong thư mục main
thêm dòng: COMPONET_EMBED_TXTFILES: = path;
\*\*\* để lấy dữ liệu từ flash:

- dùng lệnh extern:
  ++ extern const uint8_t index_html_start[] asm("\_binary_index_html_start");
  ++ extern const uint8_t index_html_end[] asm("\_binary_index_html_end");
  và lưu ý phải set type

---

để tạo key client trong MQTT:
chạy Git Bash (chuột phải bấm git bash here):
sau đó làm theo link: https://test.mosquitto.org/ssl/

---

partitions.csv: bảng chia vùng nhớ của esp
thiết lập trong menuconfig
có offset: địa chỉ đặt file. có thẻ tùy chỉnh offset và size
lưu ý thay đổi flash trong menuconfig

---

để có thể link thư viện bên ngoài (components) thêm dòng
set(req <tên thư viện> <tên thư viện>) trong file CMakeList trong hàm main
