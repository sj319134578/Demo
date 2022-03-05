#include <stdio.h>  //编译预处理，引用标准函数库中的输入输出
#include <stdlib.h>  //引用杂项函数及内存分配函数
#include <time.h>     //引用时间处理函数
#include <math.h>      //引用数学函数
#include <string.h>     //引用字符串处理函数

#define N 8                //定义每行的位示图的最大块数，每块128kB，一行为1024KB，也即1M，2的10次方
#define page_len  2        //页长 2KB

int b[50][5];//用于存储 页号（逻辑地址）与块号（物理地址）对应关系
int empty_block = 8;  //空块个数，页框数，这里的大小定义将决定LRU算法是否会执行以及执行的频率
int page;              //进程页数
int Ltimes;            //程序时间
int a[32]= {0},a2[24]= {0};

typedef struct page_trend
{
    int process_name;
    int page_name;
    int *next;
} pat;

/**
    进程控制块结构体
    包括：进程名称，进程剩余执行时间、进程大小以及下一个进程的地址next
*/
typedef struct PCB   //进程控制块
{
    int priority;             //进程优先级
    int name; //进程名称
    int times;//进程运行时间
    int p_prioruity;    //进程优先级
    struct PCB *next;
    int sizeKB;//进程大小
} pcb;

typedef struct pts //父链表结构
{
    int priority;           //进程优先级
    int process;//进程名称
    int p_amount;//进程页数
    struct pt *pHead;    //指向子链表头
    struct pts *next;    //指向下一个父链表
} PTS;


typedef struct pt //子链表（页表）结构
{

    int p_number;//页号
    int p_block;//块号
    int LRUtime;//进程最近被访问时间
    struct pt *next;
} pt;



//1. 初始化进程控制块，为进程的头指针申请空间并返回头指针
pcb *Init(void)
{
    pcb *head;//定义表头
    head = (pcb*)malloc(sizeof(pcb));  //申请空间
    if(head==NULL)
        printf("error!\n");

    head->next = NULL;
    return(head);
}


//3. 输入指定数量的进程的基本信息
pcb *create(pcb *head,int num)    //进程控制块头指针，进程数量
{
    int i;          //进程个数变量
    pcb *p,*q;      //创建两个进程控制块指针
    p = head;    //p指针指向进程控制块首地址。

    srand((unsigned)time(NULL));//随机数种子
    printf("输入每个进程大小：\n");
    for(i = 0; i < num; i++)
    {
        q = (pcb*)malloc(sizeof(pcb));  //为当前进程申请pcb型的堆控件
        q->name = i+1;
        q->times = 1;                  //生成进程名字
        scanf("%d",&q->sizeKB);//输入
        q->priority = q->name+5;       //设置进程优先级
        q->next = p->next;              //将进程加入进程链表
        p->next = q;
        p = q;               //指针后移
    }
    return head;

}

//2. 最后输出的页表被淘汰的页也会被打印出来，
//因为被调用过就会加入页表，而已经加入的页只会更新最近被调用时间
void output(pcb *head,int num)
{
    create(head,num);// 输入指定进程的信息
    printf("\n----------------初始化---------------\n");
    printf("进程名  进程大小（KB）\n");
    printf("------------------------------------ \n");
    pcb *p;
    p = head->next;
    while(p!=NULL)
    {
        printf("%3d     %3d  \n",p->name,p->sizeKB);
        p = p->next;
    }
}


// 4. 初始化位示图
// 因为根据计算位示图共有8196块，这里为了执行LRU算法，只打印前1行，也即只分配8个页框
void init_weishi(int g[1][N])
{
    int i,j;

    for(j = 0; j < N; j++)
    {
        if( j == 0 || j == 1)    //前两块被操作系统内核（引导区）占用
        {
            g[0][j] = 1;
        }
        else                                 //其他的初始化为0
        {
            g[0][j] = 0;
        }
    }
    empty_block -= 2;  //空块被定义为了全局变量：8,首先装入OS内核，占两个页框,空快减2
    print_weish(g);   //调用打印方法

}

// 5. 打印位示图（1024行太多，这里只打印2行）
void print_weish(int g[1][N])
{
    int i,j;
    printf("\n时刻%d：\n",Ltimes);
    printf("--------位示图---------\n");
    for(j = 0; j < N; j++)
    {
        printf(" %d ",g[0][j]);
    }
    printf("\n");
    printf("\n剩余块：%d\n",empty_block);   //输出剩余块数（剩余页框数）
    printf("\n");
}


// 6. 初始化父链表表头
PTS *Init2(void)
{
    PTS *head;//定义表头
    head = (PTS*)malloc(sizeof(PTS));
    if(head==NULL)
        printf("error!\n");

    head->next = NULL;
    return(head);
}


//7. 创建父链表
PTS *create_table(pcb *head,PTS *T_head)   //需传入父链表地址，子链表地址
{
    pcb *p,*q;      //进程指针
    PTS *tp,*tq;    //父链表指针
    pt *sHead;      //子链表指针

    tp=T_head;         //父链表首地址
    p=head;            //进程链表首地址
    q=p->next;

    while(q!=NULL)         //如果进程不为空
    {
        tq = (PTS*)malloc(sizeof(PTS));   //申请空间

        tq->process=q->name;   //计算进程分页数量
        if(q->sizeKB%2 == 0)        //进程大小是否能被页大小整除
        {
            tq->p_amount=q->sizeKB/page_len;
        }
        else                   //如果不能整除，则要多分一页
            tq->p_amount=(q->sizeKB/page_len)+1;
        sHead = (pt*)malloc(sizeof(pt));      //申请空间

        tq->pHead=sHead;                //将子链表的首地址赋给父链表指向的子链表地址
        tq->pHead->next=NULL;           //末尾标注
        tq->next = tp->next;            //加入子链表地址
        tp->next = tq;
        tp = tq;                        //父链表指向的子链表地址指针后移
        q=q->next;                      //进程指针后移
    }
    print_table_one(T_head);            //打印父链表信息
    return T_head;
}

// 8. 打印父链表
void print_table_one(PTS *T_head)
{
    PTS *p;
    p=T_head->next;
    printf("\n每页2kb，则\n进程名  进程页数\n");
    while(p!=NULL)
    {
        printf("  %d         %d\n",p->process,p->p_amount);
        page+=p->p_amount;//计算页数
        p=p->next;
    }
}



// 创建子链
//11. 需传入页号，父链表节点地址
pt *create_page(int amount,PTS *tp,int i,int j,int g[1][N]) //tp表示父链表上的一个结点
{
    pt *p,*q;     //子链表指针
    int count2;    //当前页是否在工作集中
    p=tp->pHead;   //从父链表处获得子链表首地址
    q=p->next;     //指针后移

    int len;       //进程页数
    // int time;
    int block;     //块号
    len=tp->p_amount;   //进程页数
    //页表中已经存在该页面，只需要修改时间
    count2 = isIn(tp,amount);

    q=p->next;

    //页面不在当前的页框中存在
    while(len>0 && count2==0)
    {
        if(p->next==NULL)
        {
            block=j;
            g[i][j]=1;
            empty_block--;   //空块数量减少
            q=(pt*)malloc(sizeof(pt));   //申请空间
            q->p_number=amount;    //修改进程信息
            q->p_block=block;
            Ltimes++;              //系统时间++
            q->LRUtime=Ltimes;     //修改被调用时间
            q->next = p->next;     //将进程加入进程列表
            p->next = q;
            p = q;
            break;
        }
        else
        {
            p=p->next;   //指针后移
            len--;       //进程页数--，装入别的页
        }
    }

    return tp->pHead;
}
// 12. 判断页面是否放入
int isIn(PTS *tp,int amount)          //需传入父链表地址，页号
{
    pt *p,*q;  //定义两个子链表指针，
    int count2=0;

    p=tp->pHead;     //指针从父链表指向子链表头
    q=p->next;       //q指针为p指针后移一位
    while(q!=NULL)    //如果父链表中有子链表
    {

        if(q->p_number==amount)             //进程页数
        {
            count2++;                       //进程装入标志
            Ltimes++;                       //程序时间++
            q->LRUtime=Ltimes;              //进程最近访问时间
            printf("第%d页已经装入\n",amount);
            printf("修改最近访问时间为：%d\n",Ltimes);
            break;
        }
        else
            q=q->next;            //指针后移
    }
    return count2;        //返回1为已经装入，返回0为没有装入
}

// 9. 初始化页表
void init_pageTable(pcb *head,PTS *T_head,int num,int g[1][N])
{
    PTS *p,*q;
    int process_number;//进程名称
    int amount;//页数
    int i,j;
    p = T_head;
    //进程号只有随机生成才能执行全局替换LRU算法
    process_number=Sort1(head); //随机生成要执行的进程号[1,num]
    printf("\n当前要执行进程%d\n",process_number);
    p=T_head;
    q=p->next;
    while(q!=NULL)         //如果父链表不为空
    {
        if(q->process==process_number)    //如果当前父链表指向的是当前要执行的进程
        {
            amount=rand()%q->p_amount;//[0,p_amount-1] 随机挑出该进程中的一页进行执行。
            printf("当前执行页%d\n",amount);

            if(empty_block>0)         //如果当前空块（页框）仍有剩余
            {

                for(int c = 2; c < N; c++) //从2开始查找空块，0,1已经被OS内核占用
                {
                    i = 0;
                    j = c;    //列数
                    if(g[0][j] == 0)
                    {
                        create_page(amount,q,i,j,g);
                        break;
                    }
                }
            }

            else //没有空块，执行置换算法
            {
                int count1;
                count1 = isIn(q,amount);//判断当前页是否已经在页框中，若不在则需置换，返回0
                if(count1 == 0)
                {
                    printf("\n…………………………此时需执行LRU进行页面替换！……………………\n");
                    LRU(g,T_head);
                    printf("………………………………页面替换完成………………………………\n");
                    printf("\n");


                    for(int c = 2; c < N; c++)    //将替换进入的进程装入
                    {
                        i = 0;
                        j = c;
                        if(g[i][j] == 0)
                        {
                            create_page(amount,q,i,j,g);
                            break;
                        }
                    }
                }

            }

        }
        q=q->next;
    }

}

//10. 对进程的优先级进行排序
int Sort1(pcb *head)    //参数为头指针，传值时传入头指针的地址
{
    pcb *q, *p, *tail, *temp;
    tail = NULL;   //将链表的尾部置为NULL
    q = head;      //将链表的头指针地址赋值给q，从而进行操作

    while((head->next) != tail)     //如果还没跑到链表的尾部，则继续执行
    {
        p = head->next;
        q = head;
        while(p->next != tail)          //链表没跑到尾部，继续执行
        {
            //将优先级低的进程放到链表的尾部
            if((p->priority) > (p->next->priority))    //如果p的优先级没有下一个节点的优先级高（优先级的数字越小，优先级越高）
            {
                q->next = p->next;
                temp = p->next->next;
                p->next->next = p;
                p->next = temp;
                p = q->next;
            }
            p = p->next;     //指针后移，继续比较
            q = q->next;
        }
        tail = p;
    }
    //相当于一个冒泡排序
    head->next->priority+=1;
    return head->next->name;
}


// 13. LRU淘汰算法
void LRU(int g[1][N],PTS *T_head) //只要使用过就放到队尾，优先淘汰链表前面的
{
    PTS *p,*q,*r;
    pt *pp,*qq;
    pt *pp1,*qq1;
    p=T_head;
    q=p->next;
    r=q;
    int shortime=100;
    int process;
    while(q != NULL)    //找出距离上次访问间隔最久的进程
    {
        pp=q->pHead;
        qq=pp->next;
        while(qq!=NULL)
        {
            if(qq->LRUtime<shortime)
            {
                shortime=qq->LRUtime;
                process=q->process;
            }

            qq=qq->next;
        }
        q = q->next;
    }
    printf("当前需置换：进程 %d中的\n",process);    //将列表最前面的进程置换出去。
    while(r!=NULL)
    {
        if(r->process==process)     //如果当前指针指向的进程是要被置换的进程则进行置换，否则指针后移
        {
            pp1=r->pHead;   //页表首地址
            qq1=pp1->next;

            while(qq1!=NULL)    //页表链表不为空
            {

                if(qq1->LRUtime==shortime)
                {

                    int p_block;
                    int i,j;
                    p_block=qq1->p_block;
                    printf("          页号 %d,块号 %d   \n",qq1->p_number,qq1->p_block);
                    i=0;
                    j=p_block;

                    Ltimes++;
                    qq1->LRUtime = Ltimes;
                    g[i][j]=0;
                    empty_block++;
                    printf("\n****LRU位示图****\n");
                    print_weish(g);

                    break;
                }
                qq1=qq1->next;
            }
            break;
        }

        r=r->next;
    }
}


// 14. 打印
void print_table2(PTS *T_head)
{
    PTS *p;
    pt *pp;
    p=T_head->next;

    while(p!=NULL)
    {
        printf("\n进程名  进程页数  \n");

        printf(" %d         %d\n",p->process,p->p_amount);
        printf("页号  块号  最近使用时间\n");
        pp=p->pHead->next;
        while(pp!=NULL)
        {

            printf(" %2d     %2d      %2d\n",pp->p_number,pp->p_block,pp->LRUtime);
            pp=pp->next;
        }
        p=p->next;
    }
}

// 15. 存地址
int serve(PTS *T_head)          //需传入父链表地址，页号
{
    PTS *p;
    pt *pp;
    p=T_head->next;
    int nn1=0;

    while(p!=NULL)
    {
        pp=p->pHead->next;
        while(pp!=NULL)
        {
            b[nn1][0]=pp->p_number;//存储页号
            b[nn1][1]=pp->p_block;//块号
            b[nn1][2]=pp->LRUtime;
            pp=pp->next;
            nn1++;
        }
        p=p->next;
    }

    printf("页号  块号  \n");
    for(int i=0; i<nn1; i++)
    {
        printf(" %2d     %2d\n",b[i][0],b[i][1]);

    }
}
//16. 由页号（逻辑地址）查块号（物理地址）
int change(int h)
{
    for(int i=0; i<50; i++)
    {
        if(b[i][0]==h)
            return b[i][1];//返回块号
    }
    return 0;
}
int main()
{
    pcb *head;
    PTS *thead;
    int num;
    int flag=0;
    int G[1][N];
    int i;
    printf("             -------------------------------************************------------------------------\n");
    printf("             -------------------------------简单虚拟分页存储管理系统------------------------------\n");
    printf("             -------------------------------************************--------------------------\n");
    head = Init();//初始化表头
    while(flag!=1)
    {
        flag=1;
        printf("\n 请输入进程数目:");
        scanf("%d",&num);
        if(num > 10)          //进程个数超过最大并发度，修正为10并提醒用户
        {
            printf("并发度最大为10，请重新输入进程数目！");
            flag=0;
        }
    }

    output(head,num);     //输入生成的进程
    Ltimes=0;
    init_weishi(G);       //初始化位示图
    thead=Init2();
    create_table(head,thead);   //创建父链表（进程名称、进程页数（根据计算得出）、进程对应的页表地址）

    printf("\n要处理页的个数为：%d\n",page);   //输出要处理的进程页数之和（page为全局变量）
    printf("--------------------------------------------------------\n\n\n");
    for(i=0; i<page; i++)
    {
        init_pageTable(head,thead,num,G);    //调页
        print_weish(G);              //打印位示图观察执行情况
        print_table2(thead);     //打印进程被调页的情况（这里的调页每个进程由于随机调页所以打印的输出的页表不会涵盖所有的页）
        printf("--------------------------------------------------------\n\n\n");
    }

    serve(thead);// 将对应关系存在数组中


    int num2;
    printf("\n\n-------------逻辑地址变物理地址----------\n");
    printf("输入一个十进制逻辑地址(如3208):\n");
    scanf("%d",&num2);
    //10进制转2进制
    for(int i=0; i<32; i++) //初始化逻辑地址数组
    {
        a[i]=0;
    }
    for(int i=0; i<24; i++) //初始化逻辑地址数组
    {
        a2[i]=0;
    }
    i=31;
    int k=23;
    int count3=0;
    while (num2>0)   //循环过程的始终，判断num2是否能被2除尽
    {
        a[i] = num2%2;   //用数组存储每次除以2之后的余数，即断定奇偶性，对应二进制位上数值
        if(i>=21)    a2[k]=a[i];//后11位页内地址相同
        i --;
        k--;
        num2 = num2/2;
        count3++;
    }
    printf("\n\n逻辑地址十进制转换为二进制数是:\n");
    for(int j=0; j < 32; j++)
    {
        printf("%d",a[j]);
        if((j+1)%4==0) printf(" ");

    }
    printf("\n\n对应11位页内地址二进制数是:\n");
    for(int j=21; j < 32; j++)
    {
        printf("%d",a[j]);
        if((j+1)%4==0) printf(" ");

    }

    //printf("逻辑地址共%d位\n",count3);

    int pa;//记录页号
    pa=a[17]*8+a[18]*4+a[19]*2+a[20];
    printf("\n则对应页号为：%d\n",pa);

    int h = change(pa);//记录块号
    if(h!=0) //若查询到对应物理地址，则输出
    {
        int i2=12;//从数组下标23-11=12开始
        printf("\n\n对应块号为:%d\n",h);
        while (h>0)   //循环过程的始终，判断num2是否能被2除尽
        {
            a2[i2] = h%2;   //用数组存储每次除以2之后的余数，即断定奇偶性，对应二进制位上数值
            h = h/2;
            i2--;
        }

        printf("\n\n对应物理地址二进制数是:\n");
        for(int j=0; j < 24; j++)
        {
            printf("%d",a2[j]);
            if((j+1)%4==0) printf(" ");

        }
    }
    else
        printf("\n未查找到对应物理地址！");
    printf("\n");
    return 0;
}

