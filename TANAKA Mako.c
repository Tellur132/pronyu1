
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define MAX_HP 100
#define DECK_SIZE 52
#define HAND_SIZE 5
#define CARDS_PER_ROUND (HAND_SIZE * 2 + 1)

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
static void show_hand(const Card hand[]);
static const char *suit_bg_color(char mark);
static void print_card(const Card *card);
static int evaluate_hand(const Card hand[]);
static void print_intro(void);
static void reset_deck(Card deck[]);
static void deal_from_position(Card deck[], int *pos, Player *player, Player *cpu, Card *player_extra, Card *cpu_extra);
static void cpu_exchange(Player *cpu, Card *extra);
static void clear_screen(void);
static void print_status(int round, const Player *player, const Player *cpu);

static int deck_pos = 0;

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

/* 山札を初期化してシャッフルし、使用位置をリセットする */
static void reset_deck(Card deck[])
{
    init_deck(deck);
    shuffle_deck(deck);
    deck_pos = 0;
}


/* デッキの現在位置からカードを配る */
/* デッキの現在位置からカードを配る */
static void deal_from_position(Card deck[], int *pos, Player *player, Player *cpu, Card *player_extra, Card *cpu_extra)
{
    for (int i = 0; i < HAND_SIZE; ++i)
    {
        player->hand[i] = deck[*pos];
        (*pos)++;
    }
    for (int i = 0; i < HAND_SIZE; ++i)
    {
        cpu->hand[i] = deck[*pos];
        (*pos)++;
    }
    if (player_extra)
    {
        *player_extra = deck[*pos];
        (*pos)++;
    }
    if (cpu_extra)
    {
        *cpu_extra = deck[*pos];
        (*pos)++;
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
    int suit_count[4] = {0}; /* H, D, C, S の順でスート数を数える */
    for (int i = 0; i < HAND_SIZE; ++i)
    {
        count[hand[i].number]++;
        switch (hand[i].mark)
        {
            case 'H':
                suit_count[0]++;
                break;
            case 'D':
                suit_count[1]++;
                break;
            case 'C':
                suit_count[2]++;
                break;
            case 'S':
                suit_count[3]++;
                break;
        }
    }
    int pairs = 0;
    int three = 0;
    int four = 0;
    int flush = 0;      /* 全て同じスート */
    int four_suit = 0;  /* 4枚同じスート */

    for (int i = 0; i < 4; ++i)
    {
        if (suit_count[i] == HAND_SIZE)
        {
            flush = 1;
        }
        else if (suit_count[i] == HAND_SIZE - 1)
        {
            four_suit = 1;
        }
    }
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
    else if (flush)
    {
        return 30;
    }
    else if (four_suit)
    {
        return 12;
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
    printf("    フラッシュ (同じスート5枚): 30ダメージ\n");
    printf("    スリーカード (同じ数字3枚): 20ダメージ\n");
    printf("    ツーペア (同じ数字2枚が2組): 15ダメージ\n");
    printf("    フォーカード(色) (同じスート4枚): 12ダメージ\n");
    printf("    ワンペア (同じ数字2枚): 10ダメージ\n");
    printf("    役なし: 5ダメージ\n");
    printf("各ラウンドで1枚だけ手札を交換することができます。\n");
    printf("HPが0になった方が負けです。\n");
    printf("-------------------------------------------\n");
    printf("エンターキーを押してゲームを開始します\n");
}

/* CPUが1枚カードを交換する */
static void cpu_exchange(Player *cpu, Card *extra)
{
    int score = evaluate_hand(cpu->hand);
    int number_count[14] = {0};
    int suit_count[4] = {0};
    for (int i = 0; i < HAND_SIZE; ++i)
    {
        number_count[cpu->hand[i].number]++;
        switch (cpu->hand[i].mark)
        {
            case 'H':
                suit_count[0]++;
                break;
            case 'D':
                suit_count[1]++;
                break;
            case 'C':
                suit_count[2]++;
                break;
            case 'S':
                suit_count[3]++;
                break;
        }
    }

    int candidates[HAND_SIZE];
    int cand_count = 0;

    if (score >= 30 && score != 40)
    {
        /* フラッシュやフルハウス以上は崩さない */
        return;
    }

    if (score == 40)
    {
        for (int i = 0; i < HAND_SIZE; ++i)
        {
            if (number_count[cpu->hand[i].number] == 1)
            {
                candidates[cand_count++] = i;
            }
        }
    }
    else if (score == 20)
    {
        for (int i = 0; i < HAND_SIZE; ++i)
        {
            if (number_count[cpu->hand[i].number] != 3)
            {
                candidates[cand_count++] = i;
            }
        }
    }
    else if (score == 15)
    {
        for (int i = 0; i < HAND_SIZE; ++i)
        {
            if (number_count[cpu->hand[i].number] == 1)
            {
                candidates[cand_count++] = i;
            }
        }
    }
    else if (score == 12)
    {
        char flush_suit = 0;
        char suits[4] = {'H', 'D', 'C', 'S'};
        for (int s = 0; s < 4; ++s)
        {
            if (suit_count[s] == 4)
            {
                flush_suit = suits[s];
                break;
            }
        }
        for (int i = 0; i < HAND_SIZE; ++i)
        {
            if (cpu->hand[i].mark != flush_suit)
            {
                candidates[cand_count++] = i;
            }
        }
    }
    else if (score == 10)
    {
        for (int i = 0; i < HAND_SIZE; ++i)
        {
            if (number_count[cpu->hand[i].number] == 1)
            {
                candidates[cand_count++] = i;
            }
        }
    }
    else
    {
        for (int i = 0; i < HAND_SIZE; ++i)
        {
            candidates[cand_count++] = i;
        }
    }

    if (cand_count == 0)
    {
        return;
    }

    int idx = candidates[rand() % cand_count];
    Card old = cpu->hand[idx];
    cpu->hand[idx] = *extra;

    printf("CPUがカードを交換しました\n");
    printf("CPU: ");
    print_card(&old);
    printf(" -> ");
    print_card(extra);
    printf("\n");
}

/* 画面をクリアする */
static void clear_screen(void)
{
    /* ANSI escape sequence to clear screen and move cursor */
    printf("\x1b[2J\x1b[H");
}

/* ラウンド情報とHPを表示する */
static void print_status(int round, const Player *player, const Player *cpu)
{
    clear_screen();
    printf("===========================================\n");
    printf("\t\t\x1b[1mポーカーバトル\x1b[0m\n");
    printf("=============== Round %2d ================\n", round);
    printf("あなたのHP: %3d\tCPUのHP: %3d\n", player->hp, cpu->hp);
    printf("-------------------------------------------\n");
}

int main(void)
{
    srand(time(NULL));

    Card deck[DECK_SIZE];
    Player player = {.hp = MAX_HP};
    Player cpu = {.hp = MAX_HP};

    print_intro();
    getchar();

    int use_persistent_deck = 0;
    int shuffle_threshold = 0;
    printf("カードを使い切るまで再利用しないモードにしますか? (1:する 0:しない) : ");
    scanf("%d", &use_persistent_deck);
    if (use_persistent_deck)
    {
        printf("山札の残り枚数がいくつを下回ったらシャッフルしますか? : ");
        scanf("%d", &shuffle_threshold);
    }
    getchar();

    /* 初回の山札準備 */
    reset_deck(deck);

    int round = 1;
    while (player.hp > 0 && cpu.hp > 0)
    {
        if (!use_persistent_deck)
        {
            reset_deck(deck);
        }
        else
        {
            int remaining = DECK_SIZE - deck_pos;
            if (remaining < CARDS_PER_ROUND || remaining < shuffle_threshold)
            {
                printf("山札をシャッフルします\n");
                reset_deck(deck);
            }
        }

        Card player_extra;
        Card cpu_extra;
        deal_from_position(deck, &deck_pos, &player, &cpu, &player_extra, &cpu_extra);

        int player_choice;

        print_status(round, &player, &cpu);
        printf("あなたの手札 : ");
        show_hand(player.hand);
        printf("\n");
        printf("CPUの手札 : ");
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

        cpu_exchange(&cpu, &cpu_extra);

        int player_damage = evaluate_hand(player.hand);
        int cpu_damage = evaluate_hand(cpu.hand);

        cpu.hp -= player_damage;
        player.hp -= cpu_damage;

        printf("あなたの攻撃: %dダメージ\n", player_damage);
        printf("CPUの攻撃: %d ダメージ\n", cpu_damage);
        printf("あなたのHP : %d\n", player.hp);
        printf("CPUのHP : %d\n", cpu.hp);
        printf("-------------------------------------------\n");
        printf("エンターキーを押して次のラウンドへ\n");
        getchar();
        getchar();
        round++;
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
