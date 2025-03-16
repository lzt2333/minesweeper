#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <easyx.h>
#include <windows.h> // ����Windows�ض���ͷ�ļ�

#define ROW 10 // �������еĳ���
#define COL 10
#define MineNum 10 // �׵�����
#define ImgSize 40
#define TOP_RANKS 10 // ���а�����

IMAGE imgs[12];
char title[50];
clock_t startTime, endTime; // ��ʱ����
struct RankEntry {
    double time;
    char date[30];
};
RankEntry topRanks[TOP_RANKS]; // �洢ǰʮ����ʱ�������
bool gameOver = false; // ��־��������ʾ��Ϸ�Ƿ��Ѿ�����

void init(int map[][COL]); // ��ʼ������
void show(int map[][COL]); // ����������ʾ
void loadResource(); // ����ͼ
void draw(int map[][COL]); // ��ͼ
void mouseMsg(ExMessage* msg, int map[][COL]); // ��������� ��� �㿪
void boomBlank(int map[][COL], int row, int col); // ը���հ׵���
int judge(int map[][COL], int row, int col); // �ж���Ӯ
void startTimer(); // ��ʼ��ʱ
void stopTimer(); // ֹͣ��ʱ������ʱ���
char* displayTime(); // ������Ϸ��ʱ�ַ���
char* getRankString(); // ��ȡ���а��ַ���
void updateRank(double time); // �������а�
void saveRankings(); // �������а��ļ�
void loadRankings(); // ���ļ��������а�
void drawMenu(); // ���Ʋ˵�
int handleMenuClick(int x, int y); // ����˵�����¼�
void drawRanking(); // �������а�
int handleRankingClick(int x, int y); // �������а����¼�

// ��ʼ������
void init(int map[][COL]) {
    loadResource();
    // ��mapȫ����ʼ��Ϊ0
    memset(map, 0, sizeof(int) * ROW * COL);
    for (int i = 0; i < MineNum;) {
        int r = rand() % ROW;
        int c = rand() % COL;
        if (map[r][c] == 0) {
            map[r][c] = -1;
            i++;
        }
    }
    // �����γɵľŹ���������ⶼ��1��
    for (int i = 0; i < ROW; i++) {
        for (int j = 0; j < COL; j++) {
            if (map[i][j] == -1) {
                for (int r = i - 1; r <= i + 1; r++) {
                    for (int c = j - 1; c <= j + 1; c++) {
                        if ((r >= 0 && r < ROW && c >= 0 && c < COL)  // ûԽ��
                            && map[r][c] != -1)
                            ++map[r][c];
                    }
                }
            }
        }
    }
    // ���ܸ���
    for (int i = 0; i < ROW; i++) {
        for (int k = 0; k < COL; k++) {
            map[i][k] += 20;
        }
    }
}

// ����������ʾ
void show(int map[][COL]) {
    for (int i = 0; i < ROW; i++) {
        for (int k = 0; k < COL; k++) {
            printf("%2d ", map[i][k]);
        }
        printf("\n");
    }
}

// ����ͼ
void loadResource() {
    for (int i = 0; i < 12; i++) {
        char imgPath[50] = { 0 };
        sprintf(imgPath, "./images/%d.jpg", i);
        loadimage(&imgs[i], imgPath, ImgSize, ImgSize);
    }
}

// ���� ��ͼ
void draw(int map[][COL]) {
    // ��ͼ
    for (int i = 0; i < ROW; i++) {
        for (int j = 0; j < COL; j++) {
            if (map[i][j] >= 0 && map[i][j] <= 8)
                putimage(j * ImgSize, i * ImgSize, &imgs[map[i][j]]);
            else if (map[i][j] == -1)
                putimage(j * ImgSize, i * ImgSize, &imgs[9]);
            else if (map[i][j] >= 19 && map[i][j] <= 28) {
                putimage(j * ImgSize, i * ImgSize, &imgs[10]);
            }
            else if (map[i][j] >= 39) {
                putimage(j * ImgSize, i * ImgSize, &imgs[11]);
            }
        }
    }
}

// ����������
void mouseMsg(ExMessage* msg, int map[][COL]) {
    // �ȸ�������������������Ӧ��������±�
    int r = msg->y / ImgSize;
    int c = msg->x / ImgSize;
    // ����򿪸���
    if (msg->message == WM_LBUTTONDOWN) {
        // ʲôʱ������Ч�ܹ��򿪣�û�д�ʱ���Դ򿪣�
        if (map[r][c] >= 19 && map[r][c] <= 28) {
            map[r][c] -= 20;
            boomBlank(map, r, c);   // ����Ƿ�հ׸��ӣ��Ǽ�ը�������Ǽ��˳�
            if (startTime == 0) {
                startTimer(); // ��Ϸ��ʼʱ������ʱ��
            }
        }
    }
    // �Ҽ���Ǹ���
    else if (msg->message == WM_RBUTTONDOWN) {
        // �Ƿ��ܹ���ǣ����û�д򿪾��ܹ����
        if (map[r][c] >= 19 && map[r][c] <= 28) {
            map[r][c] += 20;
        }
        else if (map[r][c] >= 39) {
            map[r][c] -= 20;
        }
    }
}

// ����հ׸�������������Χ���пհ׸��ӻ�������,row��col�ǵ�ǰ����ĸ��ӵ��±�
void boomBlank(int map[][COL], int row, int col) {
    // �ж�row colλ���ǲ��ǿհ׸���
    if (map[row][col] == 0) {
        for (int r = row - 1; r <= row + 1; r++) {
            for (int c = col - 1; c <= col + 1; c++) {
                if ((r >= 0 && r < ROW && c >= 0 && c < COL)  // ûԽ��
                    && map[r][c] >= 19 && map[r][c] <= 28) // û�д�
                {
                    map[r][c] -= 20;
                    boomBlank(map, r, c);
                }
            }
        }
    }
}

// ��Ϸ�������� ���˷���-1��û��������0��Ӯ�˷���1
int judge(int map[][COL], int row, int col) {
    // �㵽���ף����� ����
    if (map[row][col] == -1 || map[row][col] == 19) {
        return -1;
    }
    // �����˸��ӣ����� Ӯ�� �㿪��90������
    int cnt = 0;
    for (int i = 0; i < ROW; i++) {
        for (int k = 0; k < COL; k++) {
            // ͳ�ƴ򿪵ĸ��ӵ�����
            if (map[i][k] >= 0 && map[i][k] <= 8) {
                ++cnt;
            }
        }
    }
    if (ROW * COL - MineNum == cnt) {
        return 1;
    }
    return 0;
}

// ��ʼ��ʱ
void startTimer() {
    startTime = clock();
}

// ֹͣ��ʱ������ʱ���
void stopTimer() {
    endTime = clock();
}

// ������Ϸ��ʱ�ַ���
char* displayTime() {
    static char timeStr[50];
    double duration = (double)(endTime - startTime) / CLOCKS_PER_SEC;
    sprintf(timeStr, "��Ϸ��ʱ: %.2f ��\n", duration);
    return timeStr;
}

// ��ȡ���а��ַ���
char* getRankString() {
    static char rankStr[500];
    strcpy(rankStr, "");
    for (int i = 0; i < TOP_RANKS; i++) {
        if (topRanks[i].time != 0) {
            char temp[60];
            sprintf(temp, "��%d�� %.2f �� %s\n", i+1,topRanks[i].time, topRanks[i].date);
            strcat(rankStr, temp);
        }
    }
    return rankStr;
}


// �������а�
// �������а�
void updateRank(double ti) {
    time_t now = time(NULL);
    struct tm* tinfo;
    tinfo = localtime(&now);

    char formattedDate[30];
    strftime(formattedDate, sizeof(formattedDate), "%Y-%m-%d-%H:%M:%S", tinfo);

    // �ҵ��³ɼ�Ӧ�ò����λ��
    int i;
    for (i = 0; i < TOP_RANKS; i++) {
        if (topRanks[i].time == 0 || topRanks[i].time > ti) {
            break;
        }
    }

    // ����ҵ��˺��ʵ�λ�ã������³ɼ����ƶ�����Ԫ��
    if (i < TOP_RANKS) {
        for (int j = TOP_RANKS - 1; j > i; j--) {
            topRanks[j] = topRanks[j - 1];
        }
        topRanks[i].time = ti;
        strcpy(topRanks[i].date, formattedDate);
        
    }

    saveRankings(); // ���º󱣴����а��ļ�
}

// �������а��ļ�
void saveRankings() {
    FILE* file = fopen("rankings.txt", "w");
    if (file == NULL) {
        MessageBox(GetHWnd(), "�޷��������а��ļ���", "����", MB_OK);
        return;
    }
    for (int i = 0; i < TOP_RANKS; i++) {
        fprintf(file, "%.2f %s\n", topRanks[i].time, topRanks[i].date); 
    }
    fclose(file);
}

// ���ļ��������а�
void loadRankings() {
    FILE* file = fopen("rankings.txt", "r");
    if (file == NULL) {
        MessageBox(GetHWnd(), "�޷��������а��ļ������������а�", "��ʾ", MB_OK);
        memset(topRanks, 0, sizeof(topRanks));
        return;
    }
    for (int i = 0; i < TOP_RANKS; i++) {
        fscanf(file, "%lf %s", &topRanks[i].time, topRanks[i].date);
    }
    fclose(file);
}

// ���Ʋ˵�
void drawMenu() {
    setbkcolor(WHITE);
    cleardevice();

    // ���Ʊ���
    setbkmode(TRANSPARENT); 
    settextcolor(BLACK);
    settextstyle(50, 0, _T("Arial"));
    outtextxy(ImgSize * 3-5, ImgSize * 2, _T("ɨ����Ϸ"));

    // ���ư�ť����
    setfillcolor(LIGHTGRAY);
    fillrectangle(ImgSize * 3, ImgSize * 4, ImgSize * 7, ImgSize * 5);
    fillrectangle(ImgSize * 3, ImgSize * 6, ImgSize * 7, ImgSize * 7);
    fillrectangle(ImgSize * 3, ImgSize * 8, ImgSize * 7, ImgSize * 9);

    // ���ư�ť�ı�
    settextcolor(BLACK);
    settextstyle(30, 0, _T("Arial"));
    outtextxy(ImgSize * 4-10, ImgSize * 4 + 10, _T("��ʼ��Ϸ"));
    outtextxy(ImgSize * 4+5, ImgSize * 6 + 10, _T("���а�"));
    outtextxy(ImgSize * 4-10, ImgSize * 8 + 10, _T("�˳���Ϸ"));
}

// ����˵�����¼�
int handleMenuClick(int x, int y) {
    if (x >= ImgSize * 3 && x <= ImgSize * 7 && y >= ImgSize * 4 && y <= ImgSize * 5) {
        return 1; // ��ʼ��Ϸ
    }
    else if (x >= ImgSize * 3 && x <= ImgSize * 7 && y >= ImgSize * 6 && y <= ImgSize * 7) {
        return 2; // ��ʾ���а�
    }
    else if (x >= ImgSize * 3 && x <= ImgSize * 7 && y >= ImgSize * 8 && y <= ImgSize * 9) {
        return 0; // �˳���Ϸ
    }
    return -1; // δ�����ť
}

// �������а�
void drawRanking() {
    setbkcolor(WHITE);
    cleardevice();

    // ���Ʊ���
    settextcolor(BLACK);
    settextstyle(30, 0, _T("Arial"));
    outtextxy(ImgSize * 5 - textwidth(_T("���а�")) / 2, ImgSize * 1, _T("���а�"));

    // �������а�����
    settextstyle(15, 0, _T("Arial"));
    int lineSpacing = 20; // ���м��
    for (int i = 0; i < TOP_RANKS; i++) {
        if (topRanks[i].time != 0) {
            char rankStr[60];
            sprintf(rankStr, "��%d�� %.2f ��  %s", i + 1, topRanks[i].time, topRanks[i].date);
            outtextxy(ImgSize * 5 - textwidth(_T(rankStr)) / 2, lineSpacing * (i + 4), _T(rankStr));
        }
    }

    // ���㷵�ذ�ť��λ�úʹ�С��ȷ�������в�λ�ڵײ�
    int buttonWidth = ImgSize * 2;
    int buttonHeight = ImgSize * 2;
    int windowWidth = 10 * ImgSize;
    int windowHeight = 10 * ImgSize;

    int buttonX = (windowWidth - buttonWidth) / 2;
    int buttonY = windowHeight - buttonHeight - ImgSize; // ��һЩ�ռ���ײ��߾�

    // ���Ʒ��ذ�ť����
    setfillcolor(LIGHTGRAY);
    fillrectangle(buttonX, buttonY, buttonX + buttonWidth, buttonY + buttonHeight);

    // ���Ʒ��ذ�ť�ı�
    settextcolor(BLACK);
    settextstyle(30, 0, _T("Arial"));
    // �����ı�λ�ã�ʹ�����
    int textX = buttonX + (buttonWidth - textwidth(_T("����"))) / 2;
    int textY = buttonY + (buttonHeight - textheight(_T("����"))) / 2;
    outtextxy(textX, textY, _T("����"));
}

// �������а����¼�
int handleRankingClick(int x, int y) {
    // ���㷵�ذ�ť��λ�úʹ�С��ȷ�������в�λ�ڵײ�
    int buttonWidth = ImgSize * 2;
    int buttonHeight = ImgSize * 2;
    int windowWidth = 10 * ImgSize;
    int windowHeight = 10 * ImgSize;

    int buttonX = (windowWidth - buttonWidth) / 2;
    int buttonY = windowHeight - buttonHeight - ImgSize; // ��һЩ�ռ���ײ��߾�

    if (x >= buttonX && x <= buttonX + buttonWidth && y >= buttonY && y <= buttonY + buttonHeight) {
        return 1; // ���ز˵�
    }
    return -1; // δ�����ť
}



int main() {
    initgraph(ImgSize * 10, ImgSize * 10);
    srand((unsigned)time(NULL)); // �������������
    int map[ROW][COL] = { 0 };
    
    loadRankings(); // ���ļ��������а�

    //�˵�����
    bool inMenu = true;
    bool inRanking = false;
    drawMenu(); // ��ʾ�˵�
    while (inMenu|| inRanking) {
        
        if (inMenu) {
            drawMenu(); // ��ʾ�˵�
            while(inMenu){
                ExMessage msg;
                if (peekmessage(&msg, EX_MOUSE)) {
                    if (msg.message == WM_LBUTTONDOWN) {
                        int action = handleMenuClick(msg.x, msg.y);
                        if (action == 1) { // ��ʼ��Ϸ
                            init(map);
                            show(map);//��ʾ����״��
                            startTime = 0; // ���ü�ʱ��
                            gameOver = false; // ������Ϸ������־
                            inMenu = false; // ������Ϸģʽ
                        }
                        else if (action == 2) { // ��ʾ���а�
                            inMenu = false;
                            inRanking = true;
                        }
                        else if (action == 0) { // �˳���Ϸ
                            closegraph();
                            exit(0);
                        }
                    }
                }
            }
            
        }
        else if (inRanking) {

            drawRanking(); // ��ʾ���а�
            while(inRanking){
                ExMessage msg;
                if (peekmessage(&msg, EX_MOUSE)) {
                    if (msg.message == WM_LBUTTONDOWN) {
                        int action = handleRankingClick(msg.x, msg.y);
                        if (action == 1) { // ���ز˵�
                            inRanking = false;
                            inMenu = true;
                        }
                    }
                }
            }
            
        }
        else {
            draw(map); // ������Ϸ����
        }
    }

     //��Ϸ��ѭ��
    while (true) {
        // ��Ϣ��Ӧ
        ExMessage msg;
        while (peekmessage(&msg, EX_MOUSE)) {
            switch (msg.message) {
            case WM_RBUTTONDOWN:// ����Ҽ����
                mouseMsg(&msg, map);
                system("cls");
                draw(map);
                break;
            case WM_LBUTTONDOWN:  // ���������
                mouseMsg(&msg, map);
                int ret = judge(map, msg.y / ImgSize, msg.x / ImgSize); // ���֮���ж�
                if (!gameOver && (ret == -1 || ret == 1)) {
                    stopTimer(); // ��Ϸ����ʱֹͣ��ʱ
                    draw(map);
                    gameOver = true; // ������Ϸ������־
                    char resultMessage[200];
                    char title[50];
                    if (ret == -1) {
                        sprintf(resultMessage, "��Ҳ���а�С���㡫˵��������ɶʵ����?������һ�����㣿\n%s", displayTime());
                        strcpy(title, "ʧ��");
                    }
                    else if (ret == 1) {
                        double gameDuration = (double)(endTime - startTime) / CLOCKS_PER_SEC;
                        updateRank(gameDuration); // �������а�
                        sprintf(resultMessage, "������̫��ʵ���ˣ�����һ�Ѳ���\n%s", displayTime());
                        strcpy(title, "ʤ��");
                    }
                    char fullMessage[700];
                    sprintf(fullMessage, "%s\n%s", resultMessage, getRankString());
                    int select = MessageBox(GetHWnd(), fullMessage, title, MB_OKCANCEL);
                    if (select == IDOK) { // ����һ�� 
                        init(map);
                        startTime = 0; // ���ü�ʱ��
                        gameOver = false; // ������Ϸ������־
                    }
                    else if (select == IDCANCEL) { // �˳���Ϸ
                        closegraph();
                        exit(0);
                    }
                }
                system("cls");
                printf("judge=%d\n", ret);
                show(map);
                break;

            }
        }
        draw(map);
    }
    getchar();
    return 0;
}


