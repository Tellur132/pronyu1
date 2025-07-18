
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#define MAX_HP 100
#define DECK_SIZE 52
#define HAND_SIZE 5
#define MAX_EXCHANGE HAND_SIZE

/* ANSI color codes for background coloring */
#define RESET "\x1b[0m"
#define BG_RED "\x1b[41m"
#define BG_BLUE "\x1b[44m"
#define BG_GREEN "\x1b[42m"
#define BG_MAGENTA "\x1b[45m"
#define CONFIG_FILE "config.txt"

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
static void deal_from_position(Card deck[], int *pos, Player *player, Player *cpu,
                               Card player_extra[], Card cpu_extra[], int extra_count);
static void cpu_exchange(Player *cpu, Card extras[], int extra_count, int difficulty);
static void clear_screen(void);
static void print_status(int round, const Player *player, const Player *cpu);
static void title_screen(void);
static void settings_menu(int *use_persistent_deck, int *shuffle_threshold, int *flexible_exchange,
                          int *show_cpu_hand, int *penalty_divisor, int *difficulty);
static void load_config(int *use_persistent_deck, int *shuffle_threshold, int *flexible_exchange,
                        int *show_cpu_hand, int *penalty_divisor, int *difficulty);
static void save_config(int use_persistent_deck, int shuffle_threshold, int flexible_exchange,
                        int show_cpu_hand, int penalty_divisor, int difficulty);

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
static void deal_from_position(Card deck[], int *pos, Player *player, Player *cpu,
                               Card player_extra[], Card cpu_extra[], int extra_count)
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
    for (int i = 0; i < extra_count; ++i)
    {
        if (player_extra)
        {
            player_extra[i] = deck[*pos];
            (*pos)++;
        }
    }
    for (int i = 0; i < extra_count; ++i)
    {
        if (cpu_extra)
        {
            cpu_extra[i] = deck[*pos];
            (*pos)++;
        }
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
    int flush = 0;     /* 全て同じスート */
    int four_suit = 0; /* 4枚同じスート */

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
    printf("ゲーム開始時に交換できる枚数(1枚固定または0から5枚)を選択できます。\n");
    printf("HPが0になった方が負けです。\n");
    printf("-------------------------------------------\n");
    printf("エンターキーで戻ります\n");
}

/* CPUが指定枚数カードを交換する */
static int popcount(int x)
{
    int c = 0;
    while (x)
    {
        c += x & 1;
        x >>= 1;
    }
    return c;
}

static void cpu_exchange(Player *cpu, Card extras[], int extra_count, int difficulty)
{
    int used_idx[HAND_SIZE] = {0};
    if (difficulty == 0)
    {
        for (int e = 0; e < extra_count; ++e)
        {
            int idx;
            do
            {
                idx = rand() % HAND_SIZE;
            } while (used_idx[idx]);
            used_idx[idx] = 1;
            Card old = cpu->hand[idx];
            cpu->hand[idx] = extras[e];
            printf("CPUがカードを交換しました\n");
            printf("CPU: ");
            print_card(&old);
            printf(" -> ");
            print_card(&extras[e]);
            printf("\n");
        }
        return;
    }

    if (difficulty == 2)
    {
        Card best[HAND_SIZE];
        memcpy(best, cpu->hand, sizeof(best));
        int best_score = evaluate_hand(cpu->hand);

        int max_mask_e = 1 << extra_count;
        int max_mask_h = 1 << HAND_SIZE;
        Card tmp[HAND_SIZE];
        for (int me = 0; me < max_mask_e; ++me)
        {
            int use_e = popcount(me);
            if (use_e > extra_count || use_e > HAND_SIZE)
                continue;
            for (int mh = 0; mh < max_mask_h; ++mh)
            {
                if (popcount(mh) != HAND_SIZE - use_e)
                    continue;

                int idx = 0;
                for (int i = 0; i < HAND_SIZE; ++i)
                {
                    if (mh & (1 << i))
                    {
                        tmp[idx++] = cpu->hand[i];
                    }
                }
                for (int e = 0; e < extra_count; ++e)
                {
                    if (me & (1 << e))
                    {
                        if (idx < HAND_SIZE)
                            tmp[idx++] = extras[e];
                    }
                }
                int score = evaluate_hand(tmp);
                if (score > best_score)
                {
                    best_score = score;
                    memcpy(best, tmp, sizeof(best));
                }
            }
        }

        Card old[HAND_SIZE];
        memcpy(old, cpu->hand, sizeof(old));
        memcpy(cpu->hand, best, sizeof(best));
        printf("CPUがカードを交換しました\n");
        for (int i = 0; i < HAND_SIZE; ++i)
        {
            if (old[i].mark != cpu->hand[i].mark || old[i].number != cpu->hand[i].number)
            {
                printf("CPU: ");
                print_card(&old[i]);
                printf(" -> ");
                print_card(&cpu->hand[i]);
                printf("\n");
            }
        }
        return;
    }

    for (int e = 0; e < extra_count; ++e)
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
                if (number_count[cpu->hand[i].number] == 1 && !used_idx[i])
                {
                    candidates[cand_count++] = i;
                }
            }
        }
        else if (score == 20)
        {
            for (int i = 0; i < HAND_SIZE; ++i)
            {
                if (number_count[cpu->hand[i].number] != 3 && !used_idx[i])
                {
                    candidates[cand_count++] = i;
                }
            }
        }
        else if (score == 15)
        {
            for (int i = 0; i < HAND_SIZE; ++i)
            {
                if (number_count[cpu->hand[i].number] == 1 && !used_idx[i])
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
                if (cpu->hand[i].mark != flush_suit && !used_idx[i])
                {
                    candidates[cand_count++] = i;
                }
            }
        }
        else if (score == 10)
        {
            for (int i = 0; i < HAND_SIZE; ++i)
            {
                if (number_count[cpu->hand[i].number] == 1 && !used_idx[i])
                {
                    candidates[cand_count++] = i;
                }
            }
        }
        else
        {
            for (int i = 0; i < HAND_SIZE; ++i)
            {
                if (!used_idx[i])
                    candidates[cand_count++] = i;
            }
        }

        if (cand_count == 0)
        {
            return;
        }

        int idx = candidates[rand() % cand_count];
        used_idx[idx] = 1;
        Card old = cpu->hand[idx];
        cpu->hand[idx] = extras[e];

        printf("CPUがカードを交換しました\n");
        printf("CPU: ");
        print_card(&old);
        printf(" -> ");
        print_card(&extras[e]);
        printf("\n");
    }
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

/* タイトル画面 */
static void title_screen(void)
{
    clear_screen();
    printf("===========================================\n");
    printf("\t\t\x1b[1mポーカーバトル\x1b[0m\n");
    printf("===========================================\n");
    printf("1: ゲーム開始\n");
    printf("2: ルール説明\n");
    printf("3: 設定変更\n");
    printf("4: 終了\n");
    printf("-------------------------------------------\n");
    printf("選択肢を入力してください: ");
}

/* 設定メニュー */
static void settings_menu(int *use_persistent_deck, int *shuffle_threshold, int *flexible_exchange,
                          int *show_cpu_hand, int *penalty_divisor, int *difficulty)
{
    clear_screen();
    printf("============ 設定変更 ============\n");
    printf("カードを使い切るまで再利用しないモードにしますか? (1:する 0:しない) : ");
    scanf("%d", use_persistent_deck);
    if (*use_persistent_deck)
    {
        printf("山札の残り枚数がいくつを下回ったらシャッフルしますか? : ");
        scanf("%d", shuffle_threshold);
    }
    else
    {
        *shuffle_threshold = 0;
    }
    getchar();
    printf("交換枚数自由モードにしますか? (1:0-5枚 0:1枚のみ) : ");
    scanf("%d", flexible_exchange);
    getchar();
    printf("CPUの手札を表示しますか? (1:する 0:しない) : ");
    scanf("%d", show_cpu_hand);
    getchar();
    printf("負けた側のダメージ割り係数を入力してください(例:2) : ");
    scanf("%d", penalty_divisor);
    getchar();
    printf("難易度を選択してください (0:簡単 1:普通 2:難しい) : ");
    scanf("%d", difficulty);
    getchar();
    printf("設定を保存しました。\n");
    save_config(*use_persistent_deck, *shuffle_threshold, *flexible_exchange,
                *show_cpu_hand, *penalty_divisor, *difficulty);
    printf("エンターキーでタイトルに戻ります\n");
    getchar();
}

/* 設定ファイルの読み込み */
static void load_config(int *use_persistent_deck, int *shuffle_threshold, int *flexible_exchange,
                        int *show_cpu_hand, int *penalty_divisor, int *difficulty)
{
    FILE *fp = fopen(CONFIG_FILE, "r");
    if (fp)
    {
        if (fscanf(fp, "%d %d %d %d %d %d", use_persistent_deck, shuffle_threshold, flexible_exchange,
                   show_cpu_hand, penalty_divisor, difficulty) != 6)
        {
            *use_persistent_deck = 0;
            *shuffle_threshold = 0;
            *flexible_exchange = 0;
            *show_cpu_hand = 1;
            *penalty_divisor = 2;
            *difficulty = 1;
        }
        fclose(fp);
    }
    else
    {
        *use_persistent_deck = 0;
        *shuffle_threshold = 0;
        *flexible_exchange = 0;
        *show_cpu_hand = 1;
        *penalty_divisor = 2;
        *difficulty = 1;
    }
}

/* 設定ファイルの保存 */
static void save_config(int use_persistent_deck, int shuffle_threshold, int flexible_exchange,
                        int show_cpu_hand, int penalty_divisor, int difficulty)
{
    FILE *fp = fopen(CONFIG_FILE, "w");
    if (fp)
    {
        fprintf(fp, "%d %d %d %d %d %d\n", use_persistent_deck, shuffle_threshold,
                flexible_exchange, show_cpu_hand, penalty_divisor, difficulty);
        fclose(fp);
    }
}

int main(void)
{
    srand(time(NULL));

    Card deck[DECK_SIZE];
    Player player = {.hp = MAX_HP};
    Player cpu = {.hp = MAX_HP};

    int use_persistent_deck;
    int shuffle_threshold;
    int flexible_exchange;
    int show_cpu_hand;
    int penalty_divisor;
    int difficulty;
    load_config(&use_persistent_deck, &shuffle_threshold, &flexible_exchange,
                &show_cpu_hand, &penalty_divisor, &difficulty);

    while (1)
    {
        int choice;
        title_screen();
        scanf("%d", &choice);
        getchar();
        if (choice == 1)
        {
            break;
        }
        else if (choice == 2)
        {
            print_intro();
            getchar();
        }
        else if (choice == 3)
        {
            settings_menu(&use_persistent_deck, &shuffle_threshold, &flexible_exchange,
                          &show_cpu_hand, &penalty_divisor, &difficulty);
        }
        else if (choice == 4)
        {
            return 0;
        }
    }

    save_config(use_persistent_deck, shuffle_threshold, flexible_exchange,
                show_cpu_hand, penalty_divisor, difficulty);

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
            int extra_count = flexible_exchange ? HAND_SIZE : 1;
            int cards_needed = HAND_SIZE * 2 + extra_count * 2;
            int remaining = DECK_SIZE - deck_pos;
            if (remaining < cards_needed || remaining < shuffle_threshold)
            {
                printf("山札をシャッフルします\n");
                reset_deck(deck);
            }
        }

        int extra_count = flexible_exchange ? HAND_SIZE : 1;
        Card player_extras[MAX_EXCHANGE];
        Card cpu_extras[MAX_EXCHANGE];
        deal_from_position(deck, &deck_pos, &player, &cpu, player_extras, cpu_extras, extra_count);

        int player_choice;

        print_status(round, &player, &cpu);
        printf("あなたの手札 : ");
        show_hand(player.hand);
        printf("\n");
        printf("CPUの手札 : ");
        if (show_cpu_hand)
        {
            show_hand(cpu.hand);
        }
        else
        {
            for (int i = 0; i < HAND_SIZE; ++i)
            {
                printf("[ ? ]");
            }
        }
        printf("\n");
        if (flexible_exchange)
        {
            printf("交換したいカード番号を空白区切りで入力してください(0で終了) : ");
            char line[100];
            fgets(line, sizeof(line), stdin);
            int indices[MAX_EXCHANGE];
            int count = 0;
            char *tok = strtok(line, " \n");
            while (tok && count < extra_count)
            {
                int idx = atoi(tok);
                if (idx == 0)
                    break;
                if (idx >= 1 && idx <= HAND_SIZE)
                {
                    indices[count++] = idx - 1;
                }
                tok = strtok(NULL, " \n");
            }
            int swapped = 0;
            for (int i = 0; i < count; ++i)
            {
                Card old_card = player.hand[indices[i]];
                player.hand[indices[i]] = player_extras[i];
                printf("カードを交換しました :\n");
                print_card(&old_card);
                printf(" \342\206\222 ");
                print_card(&player_extras[i]);
                printf("\n");
                swapped = 1;
            }
            if (!swapped)
            {
                printf("交換しませんでした\n");
            }
        }
        else
        {
            printf("交換するカードを選んでください(1~5 ,0:交換しない) :");
            scanf("%d", &player_choice);
            getchar();
            if (player_choice >= 1 && player_choice <= HAND_SIZE)
            {
                Card old_card = player.hand[player_choice - 1];
                player.hand[player_choice - 1] = player_extras[0];
                printf("カードを交換しました :\n");
                print_card(&old_card);
                printf(" \342\206\222 ");
                print_card(&player_extras[0]);
                printf("\n");
            }
            else
            {
                printf("交換しませんでした\n");
            }
        }

        cpu_exchange(&cpu, cpu_extras, extra_count, difficulty);

        int player_score = evaluate_hand(player.hand);
        int cpu_score = evaluate_hand(cpu.hand);

        if (player_score > cpu_score)
        {
            int win_dmg = player_score;
            int lose_dmg = cpu_score / penalty_divisor;
            cpu.hp -= win_dmg;
            player.hp -= lose_dmg;
            printf("あなたの勝ち! CPUに%dダメージ\n", win_dmg);
            if (lose_dmg > 0)
            {
                printf("CPUからの反撃! あなたは%dダメージ\n", lose_dmg);
            }
        }
        else if (cpu_score > player_score)
        {
            int win_dmg = cpu_score;
            int lose_dmg = player_score / penalty_divisor;
            player.hp -= win_dmg;
            cpu.hp -= lose_dmg;
            printf("CPUの勝ち! あなたは%dダメージ\n", win_dmg);
            if (lose_dmg > 0)
            {
                printf("あなたの反撃! CPUは%dダメージ\n", lose_dmg);
            }
        }
        else
        {
            int damage = player_score; /* cpu_score と同じ */
            player.hp -= damage;
            cpu.hp -= damage;
            printf("引き分け! お互い%dダメージ\n", damage);
        }

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
