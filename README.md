ADC_transfer_to_PC
專案介紹
本專案主要實現 ADC 資料擷取與後端 PC 之間的通訊介面，將硬體端所擷取的數位訊號傳輸至電腦，供後續訊號處理、波形顯示及資料分析使用。
此系統可應用於生醫訊號量測平台，作為硬體電路與後端軟體之間的資料傳輸橋樑。
主要功能
- ADC 資料傳輸：接收並傳送 ADC 轉換後的數位訊號。
- 硬體與 PC 通訊：建立嵌入式系統與後端電腦的通訊介面。
- 訊號處理與分析：搭配後端 GUI 進行波形顯示與訊號分析。
- 系統驗證：協助硬體功能測試與量測結果驗證。
系統架構
ADC → ESP32 → PC → SDM_DEMO_GUI
開發環境與程式燒錄
本專案使用 Visual Studio Code (VS Code) 搭配 PlatformIO 進行韌體開發、編譯與燒錄。
基本操作流程：
1. 安裝 VS Code 與 PlatformIO IDE 擴充套件。
2. 使用 PlatformIO 開啟專案。
3. 確認 platformio.ini 中的開發板與硬體設定。
4. 編譯程式（Build）。
5. 將程式燒錄至 ESP32（Upload）。
6. 透過 Serial Monitor 確認資料傳輸情況。
詳細環境建置與燒錄教學請參考：
PlatformIO 環境建置與燒錄教學（HackMD）
搭配使用的軟體
本專案需搭配 SDM_DEMO_GUI 後端程式使用，以實現 ADC 資料接收、波形顯示及後續訊號分析。
後端 GUI 程式：
SDM_DEMO_GUI — GitHub Repository
