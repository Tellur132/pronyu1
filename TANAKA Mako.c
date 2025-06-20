
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define MAX_HP 100
#define DECK_SIZE 52
#define HAND_SIZE 5

/* ANSI color codes for background coloring */
#define RESET "\x1b[0m"
#define BG_RED "\x1b[41m"
#define BG_BLUE "\x1b[44m"
#define BG_GREEN "\x1b[42m"
#define BG_MAGENTA "\x1b[45m"

typedef struct
{
    char mark;  /* 'H','D','C','S' */
    int number; /* 1 - 13         */
} Card;

typedef struct
{
    Card hand[HAND_SIZE];
    int hp;
} Player;

static void init_deck(Card deck[]);
static void shuffle_deck(Card deck[]);
static void deal_initial_hands(Card deck[], Player *player, Player *cpu);
static void show_hand(const Card hand[]);
static const char *suit_bg_color(char mark);
static void print_card(const Card *card);
static int evaluate_hand(const Card hand[]);
static void print_intro(void);

/* デッキを初期化する */
static void init_deck(Card deck[])
{
    char marks[4] = {'H', 'D', 'C', 'S'};
    int x = 0;

    for (int i = 0; i < 4; ++i)
    {
        for (int j = 1; j <= 13; ++j)
        {
            deck[x].mark = marks[i];
            deck[x].number = j;
            x++;
        }
    }
}

/* デッキをシャッフルする */
static void shuffle_deck(Card deck[])
{
    for (int k = 51; k > 0; k--)
    {
        int l = rand() % (k + 1);
        Card temp = deck[k];
        deck[k] = deck[l];
        deck[l] = temp;
    }
}

/* 最初の手札を配る */
static void deal_initial_hands(Card deck[], Player *player, Player *cpu)
{
    for (int i = 0; i < HAND_SIZE; ++i)
    {
        player->hand[i] = deck[i];
        cpu->hand[i] = deck[i + HAND_SIZE];
    }
}

/* 手札を表示する */
static void show_hand(const Card hand[])
{
    for (int i = 0; i < HAND_SIZE; ++i)
    {
        print_card(&hand[i]);
    }
}

/* カードの模様から背景色を取得する */
static const char *suit_bg_color(char mark)
{
    switch (mark)
    {
        case 'H':
            return BG_RED;
        case 'D':
            return BG_BLUE;
        case 'C':
            return BG_GREEN;
        case 'S':
            return BG_MAGENTA;
        default:
            return "";
    }
}

/* 色付きで1枚のカードを表示する */
static void print_card(const Card *card)
{
    const char *color = suit_bg_color(card->mark);
    printf("%s[%2d]%s", color, card->number, RESET);
}

/* 手札の役を評価する */
static int evaluate_hand(const Card hand[])
{
    int count[14] = {0};
    for (int i = 0; i < HAND_SIZE; ++i)
    {
        count[hand[i].number]++;
    }
    int pairs = 0;
    int three = 0;
    int four = 0;
    for (int i = 1; i <= 13; ++i)
    {
        if (count[i] == 4)
        {
            four++;
        }
        else if (count[i] == 3)
        {
            three++;
        }
        else if (count[i] == 2)
        {
            pairs++;
        }
    }
    if (four)
    {
        return 40;
    }
    else if (three && pairs)
    {
        return 35;
    }
    else if (three)
    {
        return 20;
    }
    else if (pairs == 2)
    {
        return 15;
    }
    else if (pairs == 1)
    {
        return 10;
    }
    else
    {
        return 5;
    }
}

/* ゲームの説明を表示する */
static void print_intro(void)
{
    printf("-------------------------------------------\n");
    printf("\t\tポーカーバトル\t\t\n");
    printf("-------------------------------------------\n");
    printf("このゲームは、CPUとポーカーの手札の役の強さで戦うゲームです。\n");
    printf("お互いに5枚のカードが配られ、手札の役に応じて相手にダメージを与えます。\n");
    printf("    --------役の強さ-------：\n");
    printf("    フォーカード (同じ数字4枚): 40ダメージ\n");
    printf("    フルハウス (同じ数字3枚 + 同じ数字2枚): 35ダメージ\n");
    printf("    スリーカード (同じ数字3枚): 20ダメージ\n");
    printf("    ツーペア (同じ数字2枚が2組): 15ダメージ\n");
    printf("    ワンペア (同じ数字2枚): 10ダメージ\n");
    printf("    役なし: 5ダメージ\n");
    printf("各ラウンドで1枚だけ手札を交換することができます。\n");
    printf("HPが0になった方が負けです。\n");
    printf("-------------------------------------------\n");
    printf("エンターキーを押してゲームを開始します\n");
}

int main(void)
{
    srand(time(NULL));

    Card deck[DECK_SIZE];
    Player player = {.hp = MAX_HP};
    Player cpu = {.hp = MAX_HP};

    print_intro();
    getchar();

    while (player.hp > 0 && cpu.hp > 0)
    {
        init_deck(deck);
        shuffle_deck(deck);
        deal_initial_hands(deck, &player, &cpu);

        int player_choice;
        Card player_extra = deck[HAND_SIZE * 2];

        printf("あなたの手札 :\n");
        show_hand(player.hand);
        printf("\n");
        printf("cpuの手札 :\n");
        show_hand(cpu.hand);
        printf("\n");
        printf("1枚だけカードを交換できます(1~5 ,0:交換しない) :");
        scanf("%d", &player_choice);
        if (player_choice >= 1 && player_choice <= HAND_SIZE)
        {
            Card old_card = player.hand[player_choice - 1];
            player.hand[player_choice - 1] = player_extra;
            printf("カードを交換しました :\n");
            print_card(&old_card);
            printf(" \342\206\222 ");
            print_card(&player_extra);
            printf("\n");
        }
        else
        {
            printf("交換しませんでした\n");
        }

        int player_damage = evaluate_hand(player.hand);
        int cpu_damage = evaluate_hand(cpu.hand);

        cpu.hp -= player_damage;
        player.hp -= cpu_damage;

        printf("あなたの攻撃: %dダメージ\n", player_damage);
        printf("CPUの攻撃: %d ダメージ\n", cpu_damage);
        printf("あなたのHP : %d\n", player.hp);
        printf("CPUのHP : %d\n", cpu.hp);
        printf("---------------------------\n");
        printf("エンターキーを押して次のラウンドへ\n");
        getchar();
        getchar();
    }

    if (player.hp > 0)
    {
        printf("あなたの勝ちです!\n");
    }
    else
    {
        printf("CPUの勝ちです!\n");
    }

    return 0;
}
