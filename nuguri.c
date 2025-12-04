#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifdef _WIN32
#include "window.h"
#endif

#ifdef linux
#include "linux.h"
#endif

#ifdef __MACH__
#include "macos.h"
#endif


// 맵 및 게임 요소 정의
#define MAX_HP 3 // 최대 체력
#define MAX_ENEMIES 15 // 최대 적 수
#define MAX_COINS 30   // 최대 코인 수

// 방향키 판단을 위한 define (-32 catch 한 이후 판단)
#define UP 72
#define DOWN 80
#define LEFT 75
#define RIGHT 77


// 구조체 정의
typedef struct {
    int x, y;
    int dir; // 1: right, -1: left
} Enemy;

typedef struct {
    int x, y;
    int collected;
} Coin;


// 전역 변수
int MAX_STAGES = 1;
int* MAP_WIDTH;
int* MAP_HEIGHT;
// TEMP_WIDTH,HEIGHT 삭제 (MAP_WIDTH,HEIGHT 로 통합)
char*** map;

char DEBUGGING = 0;

int player_x, player_y; // 플레이어 위치
int stage = 0; // 현재 스테이지
int score = 0; // 점수

int hp = MAX_HP; // 플레이어 체력(초기값은 MAX_HP)

// 플레이어 상태
int is_jumping = 0;
int velocity_y = 0;
int on_ladder = 0;

// 게임 객체
Enemy enemies[MAX_ENEMIES];
int enemy_count = 0;
Coin coins[MAX_COINS];
int coin_count = 0;

// 함수 선언
void load_maps();
void init_stage();
void draw_game();
void update_game(char input);
void move_player(char input);
void move_enemies();
void check_collisions();
void setMapMemory();
void getMapSize();
void readBanner(char* str, int height);
void opening();
void mallocFree();
void beep();
void DBG(char* str); //debugging print

// 251203 setStage() 함수 삭제 (getMapSize에서 일괄처리)


// 251203 수정 완
int main() {
    srand(time(NULL));
    enable_raw_mode();
	
    opening();
	getMapSize();
	load_maps();
    init_stage();

    char c = '\0';
    int game_over = 0;
	
    while (!game_over && stage < MAX_STAGES) {
		
        if (kbhit()) {
            c = getch();
            if (c == 'q') {
                game_over = 1;
                continue;
            }
            if (c == -32) { // 화살표 입력 받기위한 (화살표 입력하면 -32가 먼저 입력된 후 72,80 등 고유 숫자가 입력됨.)
                c = getch();
            }
        } else {
            c = '\0';
        }
        update_game(c);
        draw_game();

        if (map[stage][player_y][player_x] == 'E') {
            stage++;
            load_maps();
            score += 100;
            if (stage < MAX_STAGES) {
                init_stage();
            } else {
                game_over = 1;
                printf("\x1b[2J\x1b[H");
                printf("축하합니다! 모든 스테이지를 클리어했습니다!\n");
                printf("최종 점수: %d\n", score);
            }
        }
    }

    disable_raw_mode();
    return 0;
}

// 맵 파일 로드 (251203 수정 완)
void load_maps() {
	if(DEBUGGING) DBG("load_maps() started");
	if(DEBUGGING) delay(30);
	
    FILE *file = fopen("map.txt", "r");
    if (!file) {
        perror("map.txt 파일을 열 수 없습니다.");
        exit(1);
    }
	
    int s = 0, r = 0;
    char line[100]; // getMapSize()에서 받는 최대 버퍼 크기와 일치화
    while (s < MAX_STAGES && fgets(line, sizeof(line), file)) {
		//if(DEBUGGING) DBG("in while");
		//if(DEBUGGING) delay(30);
		
        if ((line[0] == '\n') && r > 0) { // map.txt 파일은 LF만 사용하므로, \n만 확인해도 됨.
            s++;
            r = 0;
			if(DEBUGGING) DBG("in load_maps() stage plused");
			if(DEBUGGING) delay(30);
            continue;
        }else if (r < MAP_HEIGHT[s]) {
            strncpy(map[s][r], line, MAP_WIDTH[s]);
			//strncpy(char* str1, char* str2, int count) - string2의 count자를 string1에 복사
            r++;
        }
    }
    fclose(file);
	
	if(DEBUGGING) DBG("load_maps() ended");
	if(DEBUGGING) delay(500);
}

// 현재 스테이지 초기화 251203 수정 완
void init_stage() {
	if(DEBUGGING) DBG("init_stage(); started");
	if(DEBUGGING) delay(30);
	
    enemy_count = 0;
    coin_count = 0;
    is_jumping = 0;
    velocity_y = 0;

    for (int y = 0; y < MAP_HEIGHT[stage]; y++) {
        for (int x = 0; x < MAP_WIDTH[stage]; x++) {
            char cell = map[stage][y][x];
            if (cell == 'S') {
                player_x = x;
                player_y = y;
            } else if (cell == 'X' && enemy_count < MAX_ENEMIES) {
                enemies[enemy_count] = (Enemy){x, y, (rand() % 2) * 2 - 1};
                enemy_count++;
            } else if (cell == 'C' && coin_count < MAX_COINS) {
                coins[coin_count++] = (Coin){x, y, 0};
            }
        }
    }
	if(DEBUGGING) DBG("init_stage(); ended");
	if(DEBUGGING) delay(500);
}

// 게임 화면 그리기 (251203 수정 완)
void draw_game() {
	if(DEBUGGING) DBG("draw_game(); started");
	if(DEBUGGING) delay(300);
    delay(150);  //속도 조절
    clrscr();
    printf("Stage: %d | Score: %d\n", stage + 1, score);
    printf("HP: %d\n", hp); // 플레이어 체력 표시
    printf("조작: ← → (이동), ↑ ↓ (사다리), Space (점프), q (종료)\n");
	
    char display_map[MAP_HEIGHT[stage]][MAP_WIDTH[stage] + 1];
    for(int y=0; y < MAP_HEIGHT[stage]; y++) {
        for(int x=0; x < MAP_WIDTH[stage]; x++) {
            char cell = map[stage][y][x];
            if (cell == 'S' || cell == 'X' || cell == 'C') {
                display_map[y][x] = ' ';
            } else {
                display_map[y][x] = cell;
            }
        }
    }
    
    for (int i = 0; i < coin_count; i++) {
        if (!coins[i].collected) {
            display_map[coins[i].y][coins[i].x] = 'C';
        }
    }

    for (int i = 0; i < enemy_count; i++) {
        display_map[enemies[i].y][enemies[i].x] = 'X';
    }

    display_map[player_y][player_x] = 'P';

    for (int y = 0; y < MAP_HEIGHT[stage]; y++) {
        for(int x = 0; x < MAP_WIDTH[stage]; x++){
            printf("%c", display_map[y][x]);
        }
        printf("\n");
    }
	
	if(DEBUGGING) DBG("draw_game(); ended");
	if(DEBUGGING) delay(500);
}

// 게임 상태 업데이트
void update_game(char input) {
    if(DEBUGGING) DBG("update_game(); started");
	if(DEBUGGING) delay(30);
	
	move_player(input);
    move_enemies();
    check_collisions();
	
	if(DEBUGGING) DBG("update_game(); ended");
	if(DEBUGGING) delay(500);
}

// 플레이어 이동 로직 (251203 수정 완)
void move_player(char input) {
	if(DEBUGGING) DBG("move_player(); started");
	if(DEBUGGING) delay(30);
	
    int next_x = player_x, next_y = player_y;
    int i;
    char floor_tile = (player_y + 1 < MAP_HEIGHT[stage]) ? map[stage][player_y + 1][player_x] : '#';
	// 삼항연산자, (조건) ? (참일때 값) : (거짓일 때 값)
	// floor_tile : 플레이어 발 아래의 블록이 무엇인지
	
    char current_tile = map[stage][player_y][player_x];
	
    on_ladder = (current_tile == 'H');

    switch (input) {
        case LEFT:   next_x--; break;
        case RIGHT:  next_x++; break;
        case UP:     if (on_ladder) next_y--; break;
        case DOWN:   if (on_ladder && (player_y + 1 < MAP_HEIGHT[stage]) && map[stage][player_y + 1][player_x] != '#') next_y++; break;
        case ' ': // 점프
            if (!is_jumping && (floor_tile == '#' || on_ladder)) {
                is_jumping = 1;
                velocity_y = -2;
            } break;
    }

    if ((floor_tile == '#' || is_jumping == 1) && map[stage][player_y][next_x] != '#') player_x = next_x; // 땅 위에서 좌우이동
    
    if (on_ladder && (input == UP || input == DOWN)) { //사다리 위아래 이동
		if (next_y >= 0 && input == UP && map[stage][next_y][player_x] == '#'){ //사다리 끝에서 위로 이동했을 때
			player_y = next_y-1;
            is_jumping = 0;
            velocity_y = 0;
		}else if (next_y >= 0 && next_y < MAP_HEIGHT[stage] && map[stage][next_y][player_x] != '#') {
            player_y = next_y;
            is_jumping = 0;
            velocity_y = 0;
        }
    } else { // 사다리에서 위아래로 이동 중이 아닐때
        if (is_jumping) { // 점프중에~
            
            next_y = player_y + velocity_y;
            for(i = 1; player_y - i >= next_y; i++){ //점프 경로에 천장 체크
                if(map[stage][player_y - i][player_x] == '#'){ 
                    next_y = player_y - i + 1;
                    break;
                }
            }
			
            if(next_y <= 0) next_y = 1; // 올라가다 천장을 뚫거나 박히지 않게 예외처리
            velocity_y++; // 중력가속도(클수록 아래로)
			
			if (velocity_y > 0){ // 가속도가 아래로 향할 때 
				if (next_y >= MAP_HEIGHT[stage]) next_y = MAP_HEIGHT[stage]-2; // 행선지가 맵 끝보다 아래로 향할 때 (수정됨)
				else{ // 행선지가 맵 안이면
					int layer = 0;
					while (layer < next_y - player_y){// 현재 위치에서 목표 위치 사이에 블록이 있는지 확인
						if(DEBUGGING) DBG("in while");
						if(DEBUGGING) printf("at layer %d ", layer);
						if(DEBUGGING) delay(30);
						if (map[stage][player_y+layer][player_x] == '#'){
							if(DEBUGGING) printf("land detected   ");
							if(DEBUGGING) delay(30);
							next_y = player_y+layer -1;
							break;
						}
						layer++;
					}
				}
			}
			
			player_y = next_y;
			
			if ((player_y + 1 > MAP_HEIGHT[stage]) || map[stage][player_y + 1][player_x] == '#') { // 이미 땅에 닿아있을 때
                is_jumping = 0;
                velocity_y = 0;
				next_y = MAP_HEIGHT[stage]-1;
			}
			
        } else { // 점프중이 아니면~s
            if (floor_tile != '#' && floor_tile != 'H') { // 땅 위도 사다리 위도 아닐 때 (허공일 때)
				if (player_y+1 < MAP_HEIGHT[stage]) {is_jumping = 1; velocity_y = 1;} // 맵을 벗어나지 않으면 아래로 한 칸 이동
                else init_stage();
            }
        }
    }
    
	// 모든 계산 끝난 후 땅에 끼인경우 땅 위로 위치조정
	if (map[stage][player_y][player_x] == '#') player_y--;
	
    if (player_y >= MAP_HEIGHT[stage]) init_stage();
	
	if(DEBUGGING) DBG("move_player(); ended");
	if(DEBUGGING) delay(500);
}

// 적 이동 로직 (251203 수정 완)
void move_enemies() {
	if(DEBUGGING) DBG("move_enemies(); started");
	if(DEBUGGING) delay(30);
	
    for (int i = 0; i < enemy_count; i++) {
        int next_x = enemies[i].x + enemies[i].dir;
        if (next_x < 0 || next_x >= MAP_WIDTH[stage] || map[stage][enemies[i].y][next_x] == '#' || (enemies[i].y + 1 < MAP_HEIGHT[stage] && map[stage][enemies[i].y + 1][next_x] == ' ' && map[stage][enemies[i].y + 1][enemies[i].x] == '#')) { //떠있는 적이랑 걸어다닌 적 구분
            enemies[i].dir *= -1;
		}
        //enemies[i].x = next_x; // 좌우 끝칸에서 한 번 멈추지 않게 수정 //12.4 몬스터가 벽 뚫음 (임시 제거)
		
         else {
            enemies[i].x = next_x;
        }
    }
	
	if(DEBUGGING) DBG("move_enemies(); ended");
	if(DEBUGGING) delay(500);
}

// 충돌 감지 로직 (251203 테스트 완)
void check_collisions() {
	if(DEBUGGING) DBG("check_collisions(); started");
	if(DEBUGGING) delay(30);
	
    for (int i = 0; i < enemy_count; i++) {
    if (player_x == enemies[i].x && player_y == enemies[i].y) {
        // 적과 충돌: 체력 감소
        hp--;
        // 점수 패널티는 유지(원하면 제거 가능)
        score = (score > 50) ? score - 50 : 0;

        if (hp <= 0) {
            // 게임 오버 처리: 화면 정리 후 종료
            printf("\x1b[2J\x1b[H");
            printf("게임 오버! HP가 모두 소진되었습니다.\n");
            printf("최종 점수: %d\n", score);
            disable_raw_mode();
            exit(0);
        } else {
            // 체력이 남아있으면 현재 스테이지 재시작 (플레이어 위치 초기화)
            init_stage();
            return;
        }
    }
}

    for (int i = 0; i < coin_count; i++) {
        if (!coins[i].collected && ((player_x == coins[i].x && player_y == coins[i].y) || (is_jumping != 0 && player_y + 1 == coins[i].y && player_x == coins[i].x))) {
            beep();
            coins[i].collected = 1;
            score += 20;
        }
    }
	
	if(DEBUGGING) DBG("check_collisions(); ended");
	if(DEBUGGING) delay(500);
}

// setStage() 함수 삭제 (getMapSize에서 일괄처리)

// 맵 전역변수에 동적 메모리 할당 (251203 수정 완)
void setMapMemory() {
	if(DEBUGGING) DBG("setMapMemory(); started");
	if(DEBUGGING) delay(30);
	
    int i =0, j = 0;
	
	//전역 변수 활용하게 수정
    map = (char***)malloc(sizeof(char**) * MAX_STAGES); // MAX_STAGES
    for(i = 0; i < MAX_STAGES; i++){ 
        map[i] = (char**)malloc(sizeof(char*) * MAP_HEIGHT[i]);  //MAP_HEIGHT
        for(j = 0; j < MAP_HEIGHT[i]; j++){
            map[i][j] = (char*)malloc(sizeof(char) * MAP_WIDTH[i]); //MAP_WIDTH
        }
    }
	
	if(DEBUGGING) DBG("setMapMemory(); ended");
	if(DEBUGGING) delay(500);
}

// 맵 사이즈 계산 및 동적메모리 할당 (251203 수정 완(컴파일 테스트 필요))
void getMapSize() {
	if(DEBUGGING) DBG("getMapSize(); started");
	if(DEBUGGING) delay(30);
	
    char buffer[100];

    FILE* file = fopen("map.txt", "r");
    if (!file) {
        perror("map.txt 파일을 열 수 없습니다.");
        exit(1);
    }
	
	// MAX_STAGES 먼저 계산 
	while(fgets(buffer, sizeof(buffer), file) != NULL)
		if (buffer[0] == '\n') MAX_STAGES++;
	
	if(DEBUGGING) printf("///MAX_STAGES: %d   ", MAX_STAGES);
	if(DEBUGGING) delay(30);
	
	// MAP_WIDTH, MAP_HEIGHT 에 동적 메모리 할당
	MAP_HEIGHT = (int*)malloc(sizeof(int)*MAX_STAGES);
	MAP_WIDTH = (int*)malloc(sizeof(int)*MAX_STAGES);

	// 같은 file 다시 불러오기
	file = fopen("map.txt", "r");
    if (!file) {
        perror("map.txt 파일을 열 수 없습니다.");
        exit(1);
    }
	
	// 스테이지별 맵 크기 계산 및 저장
	int s = 0;
	int height = 0;
	while(fgets(buffer, sizeof(buffer), file) != NULL){
		if (buffer[0] == '\n'){
			MAP_HEIGHT[s] = height;
			height=0;
			s++;
		}else{
			height++;
			MAP_WIDTH[s] = strlen(buffer)-1;
			// 개행 문자 고려해 가로 길이 -1
		}
	}
	MAP_WIDTH[s] = strlen(buffer);
	MAP_HEIGHT[s] = height;
	
    fclose(file);
	
	
    setMapMemory();
	
	if(DEBUGGING){
		s = 0;
		while(s < MAX_STAGES) printf("///in stage %d: MAP_WIDTH: %d, MAP_HEIGHT: %d   ", s++, MAP_WIDTH[s], MAP_HEIGHT[s]);
		DBG("getMapSize(); ended");
		delay(500);
	}
}

// 동적메모리 할당 해제
// 251203 수정) for문 내부 동적 메모리 사용 변수 참조를 고려해 free 순서 변경
void mallocFree() {
    int i, j;
	for (i = 0; i < MAX_STAGES; i++) {
        for (j = 0; j < MAP_HEIGHT[i]; j++) {
            free(map[i][j]);
		}
        free(map[i]);
	}
	free(map);
	
    free(MAP_HEIGHT);
    free(MAP_WIDTH);
}

// 너구리 배너 띄우려고 만든 코드
void readBanner(char* str, int height){
    FILE *file = fopen(str, "r");
    if (!file) {
		perror("파일을 열 수 없습니다.");
		exit(1);
    }
    int h = 0, r = 0;
    char C;
    char line[50];
	
	while (h<height && fgets(line, sizeof(line), file)) {
		printf(line);
		h++;
	}
	fclose(file);
}

// 엔딩화면
void ending(){
	clrscr();
	readBanner("endAni1.txt", 20);
	printf(LF LF "             >> YOU WIN <<" LF LF);
	return;
}

// 시작화면
void opening(){
	int select = 0;
	int d = 0;
	char c;
	
	clrscr();
	readBanner("banner.txt", 11);
	printf(LF LF "         press Enter to select" LF LF);
	printf("           START        EXIT");
	
	while(1){
		
		int x;
		if (select==0) x=9;
		else x=22;
		
		gotoxy(x,14);
		printf(">");
		gotoxy(0,16);
		
		c = getch();
		if (c==-32){ //화살표 입력을 받았을 때
			c = getch();
			if (c==LEFT && select!=0) {select--; gotoxy(x,14); printf(" ");}
			else if (c==RIGHT && select!=1) {select++; gotoxy(x,14); printf(" ");}
		} else if (c=='d'){ // 디버깅 모드 진입
			if (d==0) {d = 1;}
			else d = 0;
		} else if (c== LF[0]) break;
		
		if (d==1){
			gotoxy(0,12);
			printf("        Debuging mode activated" LF LF);
			printf("           START        ENDING");
		} else{
			gotoxy(0,12);
			printf("         press Enter to select" LF LF);
			printf("           START        EXIT");
		}
	}
	
	if (d==0){
		if (select == 0) return;
		else exit(1);
	}else if (d==1){
		if (select == 0){ //debug start
			DEBUGGING = 1;
			return;
		} else{ //ending
			ending();
			return;
		}
	}
}

// 디버깅 메세지 출력
void DBG(char* str){
	gotoxy(0,30);
	printf("//%s   ", str);
	return;
}

// 따로 운영체제별 분리 필요할 것으로 보임
void beep(void) {
    #ifdef _WIN32
        //윈도우
        Beep(750, 100);
    #else
        //리눅스
        printf("\a");
        fflush(stdout);
    #endif
    }

