# project_nuquri
### **OS별 컴파일/실행 방법**

##### Windows

##### Linux

##### MacOS

---

### **구현 내용**

##### 크로스 플랫폼 지원
    #ifdef 전처리 지시어를 사용하여 OS(window, linux, maxos)별로 include 할 헤더파일(termios.h, window.h 등)과 일부 함수들(getch, disable_raw_mode, goto 등)을 구분하였습니다.

##### 생명력 시스템
    #define MAX_HP로 선언하여 check_collisions 함수에서 몬스터와 충돌시 HP가 깎이고 점수 패널티 및 체력이 남아있을 시 스테이지 재시작하게 구현하였습니다

##### 타이틀 및 엔딩 화면


##### 동적 맵 할당
    getMapSize함수로 map.txt의 전체적인 스테이지 개수, 스테이지 별 높이/크기를 받아와 setMapMemory함수로 전역변수 map을 3차원 동적할당을 사용하여 스테이지 별 맞는 크기로 저장하였습니다.

##### 사운드 효과
    os별로 beep 함수를 구현하여 coin 획득시 사운드를 출력하게 구현하였습니다.

---

### **문제 해결 과정**

##### 점프 후 내려오지 않던 문제 수정
	if (velocity_y > 0 && (next_y < MAP_HEIGHT || map[stage][next_y][player_x] == '#'))을 if (velocity_y > 0 && (next_y > MAP_HEIGHT || map[stage][next_y][player_x] == '#'))으로 수정해, (next_y < MAP_HEIGHT)로 인해 velocity_y = 0; 이 항상 작동하던 부분 수정하였습니다.

##### 사다리를 끝까지 타지 못하던 문제 수정
    사다리를 타고 있고, 위쪽을 향하며, 맵 너머를 향하지 않고, 플레이어 바로 윗 칸이 #(지면) 일 때는
    사다리와 닿아있는 지면 위로 바로 올라갈 수 있도록 예외처리하였습니다.

##### 점프 시 천장에 박히는 버그 수정
    if(next_y < 0) next_y = 0;을 if(next_y <= 0) next_y = 1;으로 수정해 천장에 박히지 않게 수정하였습니다.

##### 떨어질 때 가속도로 땅 뚫는 버그 수정
    기존 복합적 상황 체크에서 (아래로 떨어지고 있을 때~) > (추가 상황) 식으로 로직 변경하고 while문 이용해 현재 위치와 목적지 사이에 벽 존재하는지 확인해서 처리하는 코드 추가하였습니다.