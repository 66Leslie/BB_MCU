# BB_MCU

Blue Bridge Cup (8051) practice and contest projects, collected by year and phase. Projects are Keil uVision builds; sources are kept, build outputs are ignored.

## 备注
- 蓝桥杯省二（自认为滑铁卢）

## Layout
- `13P`, `13_N`, `14P`, `14_N`, `15P`, `15_N`: Province (`P`) and national (`N`) problem sets with Keil projects (`.uvproj`) and source for each year.
- `points_race/p4_*`, `points_race/p5_*`: Point-race practice projects.
- `Driver`, `driver_myself`: Reusable drivers (DS1302, I2C, UART, key scan, etc.) for 8051 projects.
- `个人模版`, `模拟赛`: Personal template and mock-contest projects (Keil/EIDE).
- `个人思维导图记忆.pdf`: Personal mind-map / quick memory notes.
- `.vscode`: Editor settings for quick builds/debug (mostly Keil Assistant/EIDE logs).

## Usage
1. Clone the repo and open the desired `.uvproj` in Keil uVision (4/5).
2. Pick the target project (e.g., `13P`, `14_N`), build, and flash to your board.
3. Reuse code from `Driver` or `driver_myself` when starting new problems.

## Notes
- Build artifacts (`Objects/`, `Listings/`, `*.hex`, etc.) are ignored by `.gitignore`; rebuild locally after cloning.
- `DP2024_SCM（单片机资源包）` subfolders hold reference PDFs for segment maps.
