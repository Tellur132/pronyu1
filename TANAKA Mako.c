#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define MAXHP 100

typedef struct
{
    char mark;  //'H','D','C','S'
    int number; // 1 ~ 13
} Card;

typedef struct
{
    Card hand[5];
    int hp;
} Player;

void create(Card *deck) // deck:カードがたくさん並んでいる配列
                        //*deck:最初のカードの場所
                        // Card *deck:Card型のデータ（カード）が並んでいる場所の住所を渡す
{
    char marks[4] = {'H', 'D', 'C', 'S'};
    int x = 0;

    for (int i = 0; i < 4; i++)
    {
        for (int j = 1; j <= 13; j++)
        {
            deck[x].mark = marks[i];
            deck[x].number = j;
            x++;
        }
    }
}

void shuffle(Card *deck)
{
    for (int k = 51; k > 0; k--)
    {
        int l = rand() % (k + 1);
        Card temp = deck[k];
        deck[k] = deck[l];
        deck[l] = temp;
    }
}

void show(Card *hand) // 手札表示する関数
{
    for (int i = 0; i < 5; i++)
    {
        printf("[%c %d]", hand[i].mark, hand[i].number);
    }
}

int evaluate(Card hand[])
{
    int count[14] = {0};
    for (int i = 0; i < 5; i++)
    {
        count[hand[i].number]++;
    }
    int two = 0;
    int three = 0;
    int four = 0;
    for (int i = 1; i <= 13; i++)
    {
        if (count[i] == 2)
        {
            two++;
        }
        else if (count[i] == 3)
        {
            three++;
        }
        else
        {
            four++;
        }
    }
    if (four == 1)
    {
        return 40;
    }
    else if (three == 1 && two == 1)
    {
        return 35;
    }
    else if (three == 1)
    {
        return 20;
    }
    else if (two == 2)
    {
        return 15;
    }
    else if (two == 1)
    {
        return 10;
    }
    else
    {
        return 5;
    }
}

int main(void)
{
    srand(time(NULL));
    Card deck[52];
    Player player = {player.hp = MAXHP};
    Player cpu = {cpu.hp = MAXHP};
    printf("-------------------------------------------\n");
    printf("         ポーカーバトル       \n");
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
    getchar(); // エンターキー押したら始まる
    while (player.hp > 0 && cpu.hp > 0)
    {
        create(deck);
        shuffle(deck);
        int player_choice;
        for (int i = 0; i < 5; i++)
        {
            player.hand[i] = deck[i];
            cpu.hand[i] = deck[i + 5];
        }
        Card player_extra = deck[10];
        printf("あなたの手札 :\n");
        show(player.hand);
        printf("\n");
        printf("cpuの手札 :\n");
        show(cpu.hand);
        printf("\n");
        printf("1枚だけカードを交換できます(1~5 ,0:交換しない) :");
        scanf("%d", &player_choice);
        if (player_choice >= 1 && player_choice <= 5)
        {
            Card old_card = player.hand[player_choice-1];
            player.hand[player_choice - 1] = player_extra;
            printf("カードを交換しました :\n");
            printf("[%c %d] → [%c %d]\n",old_card.mark,old_card.number,player_extra.mark,player_extra.number);
        }
        else
        {
            printf("交換しませんでした\n");
        }

        int player_damage = evaluate(player.hand);
        int cpu_damage = evaluate(cpu.hand);
        cpu.hp = cpu.hp - player_damage;
        player.hp = player.hp - cpu_damage;
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