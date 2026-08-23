# ESP32 Image Access Point

Chương trình biến ESP32 thành một Wi-Fi Access Point độc lập. Khi điện thoại
kết nối, ESP32 cung cấp trang web hiển thị ảnh trong LittleFS. Không cần router
hoặc Internet.

## Thông tin mặc định

- Wi-Fi: `ESP32-I2C-Lesson`
- Mật khẩu: `12345678`
- Trang web: `http://192.168.4.1`
- Ảnh: `/i2c.png`

Có thể đổi Wi-Fi ở đầu `image_access_point.ino`:

```cpp
const char* AP_NAME = "ESP32-I2C-Lesson";
const char* AP_PASSWORD = "12345678";
```

Mật khẩu phải có ít nhất 8 ký tự.

## Cấu trúc thư mục bắt buộc

```text
image_access_point/
├── image_access_point.ino
├── README.md
└── data/
    └── i2c.png
```

`data` là tên thư mục đặc biệt của LittleFS uploader. Khi upload LittleFS:

```text
Máy tính: data/i2c.png
ESP32:    /i2c.png
```

Đường dẫn phải khớp với chương trình:

```cpp
const char* IMAGE_PATH = "/i2c.png";
```

## Dung lượng flash và partition

Ảnh hiện tại có kích thước 1063×1063, khoảng 64 KB. Partition LittleFS/SPIFFS
lớn hơn 64 KB là đủ.

Nếu thay bằng ảnh gốc khoảng 1.7 MB, cần partition SPIFFS ít nhất 1.8 MB. Ví dụ
trên ESP32 flash 4 MB:

```text
No OTA (2MB APP/2MB SPIFFS)
```

Không chọn:

```text
No OTA (2MB APP/2MB FATFS)
```

Chương trình dùng `LittleFS.h`, không dùng FATFS. Tên partition có thể khác tùy
board và phiên bản ESP32 board package.

## Dùng Arduino IDE 2.x

Plugin VSIX trong hướng dẫn yêu cầu Arduino IDE 2.2.1 trở lên. Kiểm tra tại
**Help → About Arduino IDE**.

Trên máy Linux hiện tại, chạy Arduino IDE 2.3.10 bằng:

```bash
chmod +x ~/Downloads/arduino-ide_2.3.10_Linux_64bit.AppImage
~/Downloads/arduino-ide_2.3.10_Linux_64bit.AppImage
```

Nếu AppImage không mở:

```bash
~/Downloads/arduino-ide_2.3.10_Linux_64bit.AppImage \
  --appimage-extract-and-run
```

Không mở biểu tượng Arduino IDE 1.8.19 cũ nếu muốn dùng plugin VSIX.

## Cài ESP32 board package

Nếu đã cài ESP32 board package thì bỏ qua phần này.

1. Mở **File → Preferences**.
2. Thêm vào **Additional Boards Manager URLs**:

   ```text
   https://espressif.github.io/arduino-esp32/package_esp32_index.json
   ```

3. Mở **Tools → Board → Boards Manager**.
4. Tìm `esp32`.
5. Cài **esp32 by Espressif Systems**.

## Cài LittleFS uploader cho Arduino IDE 2.x

1. Tải file `.vsix` mới nhất tại
   <https://github.com/earlephilhower/arduino-littlefs-upload/releases/latest>.
2. Đóng hoàn toàn Arduino IDE.
3. Trên Linux, chạy:

   ```bash
   mkdir -p ~/.arduinoIDE/plugins
   cp ~/Downloads/arduino-littlefs-upload-*.vsix \
     ~/.arduinoIDE/plugins/
   ```

4. Khởi động lại Arduino IDE 2.x.

Plugin hiện có trên máy này tại:

```text
~/.arduinoIDE/plugins/arduino-littlefs-upload-1.6.3.vsix
```

## Chọn board, cổng USB và partition

1. Kết nối ESP32 bằng cáp USB có truyền dữ liệu.
2. Mở `image_access_point.ino` trong Arduino IDE 2.x.
3. Chọn **Tools → Board** và đúng loại ESP32. Board thông dụng có thể dùng
   `ESP32 Dev Module`.
4. Chọn **Tools → Port**, ví dụ `/dev/ttyUSB0`.
5. Chọn **Tools → Flash Size** đúng với dung lượng thật của board.
6. Chọn **Tools → Partition Scheme** có vùng SPIFFS đủ lớn.

Phải giữ cùng board và partition scheme khi upload sketch lẫn LittleFS.

## Upload sketch

Upload chương trình và upload file ảnh là hai thao tác khác nhau:

```text
Upload sketch    → ghi chương trình vào app partition
Upload LittleFS  → ghi thư mục data/ vào filesystem partition
```

1. Nhấn **Verify** để biên dịch.
2. Nếu thành công, nhấn **Upload**.
3. Chờ Arduino IDE báo hoàn thành.

Nếu dừng ở `Connecting...`, giữ nút **BOOT**, bắt đầu upload lại và thả nút khi
quá trình ghi bắt đầu.

## Upload ảnh vào LittleFS

1. Đóng Serial Monitor để giải phóng cổng USB.
2. Trong Arduino IDE 2.x, nhấn `Ctrl+Shift+P` hoặc `F1`.
3. Cũng có thể mở **View → Command Palette**.
4. Tìm và chọn:

   ```text
   Upload LittleFS to Pico/ESP8266/ESP32
   ```

5. Chờ upload hoàn thành.

Nếu không có Command Palette, kiểm tra thanh tiêu đề. Arduino IDE 1.8.19 không
có chức năng này và không dùng được plugin `.vsix`.

Sau khi đổi partition scheme, phải upload lại sketch rồi upload lại LittleFS.
Nếu chỉ thay ảnh, chỉ cần upload LittleFS lại.

## Kết nối điện thoại

1. Nhấn **EN/RESET** trên ESP32.
2. Mở Serial Monitor ở `115200` baud.
3. Kết quả mong đợi:

   ```text
   ESP32 image access point started
   Wi-Fi name: ESP32-I2C-Lesson
   Open: http://192.168.4.1
   ```

4. Kết nối điện thoại vào `ESP32-I2C-Lesson`.
5. Nhập mật khẩu `12345678`.
6. Nếu điện thoại báo không có Internet, chọn tiếp tục dùng mạng này.
7. Chờ captive portal tự mở hoặc truy cập `http://192.168.4.1`.

`192.168.4.1` là địa chỉ mặc định, cố định của ESP32 trong chế độ SoftAP của
chương trình. Nếu điện thoại tự chuyển sang 4G/5G, tạm tắt mobile data.

## Lỗi chỉ hiện dấu hỏi hoặc biểu tượng ảnh hỏng

Mở trực tiếp:

```text
http://192.168.4.1/i2c.png
```

Nếu thấy:

```text
Image missing. Upload the LittleFS data folder before starting.
```

thì ảnh chưa được upload hoặc tên file không đúng.

Nếu Serial Monitor hiện:

```text
LittleFS mount failed. Upload the data folder first.
```

hãy kiểm tra:

1. Partition là SPIFFS, không phải FATFS.
2. Partition đủ lớn cho ảnh.
3. Sketch và LittleFS dùng cùng partition scheme.
4. Đã đóng Serial Monitor trước khi upload filesystem.
5. File nằm đúng tại `image_access_point/data/i2c.png`.
6. Đã nhấn RESET sau khi upload.

Nếu vừa đổi partition, upload lại sketch trước rồi upload LittleFS lần nữa.

## Arduino IDE 1.8.19

Arduino IDE 1.8.19 không có Command Palette và không dùng plugin `.vsix` dành
cho IDE 2.x. IDE 1.8.x cần plugin Java kiểu cũ với lệnh upload trong menu
**Tools**. Cách đơn giản cho project này là dùng Arduino IDE 2.3.10 theo hướng
dẫn phía trên.
