#include <stdio.h>  //����Ԥ�������ñ�׼�������е��������
#include <stdlib.h>  //������������ڴ���亯��
#include <time.h>     //����ʱ�䴦����
#include <math.h>      //������ѧ����
#include <string.h>     //�����ַ���������

#define N 8                //����ÿ�е�λʾͼ����������ÿ��128kB��һ��Ϊ1024KB��Ҳ��1M��2��10�η�
#define page_len  2        //ҳ�� 2KB

int b[50][5];//���ڴ洢 ҳ�ţ��߼���ַ�����ţ������ַ����Ӧ��ϵ
int empty_block = 8;  //�տ������ҳ����������Ĵ�С���彫����LRU�㷨�Ƿ��ִ���Լ�ִ�е�Ƶ��
int page;              //����ҳ��
int Ltimes;            //����ʱ��
int a[32]= {0},a2[24]= {0};

typedef struct page_trend
{
    int process_name;
    int page_name;
    int *next;
} pat;

/**
    ���̿��ƿ�ṹ��
    �������������ƣ�����ʣ��ִ��ʱ�䡢���̴�С�Լ���һ�����̵ĵ�ַnext
*/
typedef struct PCB   //���̿��ƿ�
{
    int priority;             //�������ȼ�
    int name; //��������
    int times;//��������ʱ��
    int p_prioruity;    //�������ȼ�
    struct PCB *next;
    int sizeKB;//���̴�С
} pcb;

typedef struct pts //������ṹ
{
    int priority;           //�������ȼ�
    int process;//��������
    int p_amount;//����ҳ��
    struct pt *pHead;    //ָ��������ͷ
    struct pts *next;    //ָ����һ��������
} PTS;


typedef struct pt //������ҳ���ṹ
{

    int p_number;//ҳ��
    int p_block;//���
    int LRUtime;//�������������ʱ��
    struct pt *next;
} pt;



//1. ��ʼ�����̿��ƿ飬Ϊ���̵�ͷָ������ռ䲢����ͷָ��
pcb *Init(void)
{
    pcb *head;//�����ͷ
    head = (pcb*)malloc(sizeof(pcb));  //����ռ�
    if(head==NULL)
        printf("error!\n");

    head->next = NULL;
    return(head);
}


//3. ����ָ�������Ľ��̵Ļ�����Ϣ
pcb *create(pcb *head,int num)    //���̿��ƿ�ͷָ�룬��������
{
    int i;          //���̸�������
    pcb *p,*q;      //�����������̿��ƿ�ָ��
    p = head;    //pָ��ָ����̿��ƿ��׵�ַ��

    srand((unsigned)time(NULL));//���������
    printf("����ÿ�����̴�С��\n");
    for(i = 0; i < num; i++)
    {
        q = (pcb*)malloc(sizeof(pcb));  //Ϊ��ǰ��������pcb�͵Ķѿؼ�
        q->name = i+1;
        q->times = 1;                  //���ɽ�������
        scanf("%d",&q->sizeKB);//����
        q->priority = q->name+5;       //���ý������ȼ�
        q->next = p->next;              //�����̼����������
        p->next = q;
        p = q;               //ָ�����
    }
    return head;

}

//2. ��������ҳ����̭��ҳҲ�ᱻ��ӡ������
//��Ϊ�����ù��ͻ����ҳ�����Ѿ������ҳֻ��������������ʱ��
void output(pcb *head,int num)
{
    create(head,num);// ����ָ�����̵���Ϣ
    printf("\n----------------��ʼ��---------------\n");
    printf("������  ���̴�С��KB��\n");
    printf("------------------------------------ \n");
    pcb *p;
    p = head->next;
    while(p!=NULL)
    {
        printf("%3d     %3d  \n",p->name,p->sizeKB);
        p = p->next;
    }
}


// 4. ��ʼ��λʾͼ
// ��Ϊ���ݼ���λʾͼ����8196�飬����Ϊ��ִ��LRU�㷨��ֻ��ӡǰ1�У�Ҳ��ֻ����8��ҳ��
void init_weishi(int g[1][N])
{
    int i,j;

    for(j = 0; j < N; j++)
    {
        if( j == 0 || j == 1)    //ǰ���鱻����ϵͳ�ںˣ���������ռ��
        {
            g[0][j] = 1;
        }
        else                                 //�����ĳ�ʼ��Ϊ0
        {
            g[0][j] = 0;
        }
    }
    empty_block -= 2;  //�տ鱻����Ϊ��ȫ�ֱ�����8,����װ��OS�ںˣ�ռ����ҳ��,�տ��2
    print_weish(g);   //���ô�ӡ����

}

// 5. ��ӡλʾͼ��1024��̫�࣬����ֻ��ӡ2�У�
void print_weish(int g[1][N])
{
    int i,j;
    printf("\nʱ��%d��\n",Ltimes);
    printf("--------λʾͼ---------\n");
    for(j = 0; j < N; j++)
    {
        printf(" %d ",g[0][j]);
    }
    printf("\n");
    printf("\nʣ��飺%d\n",empty_block);   //���ʣ�������ʣ��ҳ������
    printf("\n");
}


// 6. ��ʼ���������ͷ
PTS *Init2(void)
{
    PTS *head;//�����ͷ
    head = (PTS*)malloc(sizeof(PTS));
    if(head==NULL)
        printf("error!\n");

    head->next = NULL;
    return(head);
}


//7. ����������
PTS *create_table(pcb *head,PTS *T_head)   //�贫�븸�����ַ���������ַ
{
    pcb *p,*q;      //����ָ��
    PTS *tp,*tq;    //������ָ��
    pt *sHead;      //������ָ��

    tp=T_head;         //�������׵�ַ
    p=head;            //���������׵�ַ
    q=p->next;

    while(q!=NULL)         //������̲�Ϊ��
    {
        tq = (PTS*)malloc(sizeof(PTS));   //����ռ�

        tq->process=q->name;   //������̷�ҳ����
        if(q->sizeKB%2 == 0)        //���̴�С�Ƿ��ܱ�ҳ��С����
        {
            tq->p_amount=q->sizeKB/page_len;
        }
        else                   //���������������Ҫ���һҳ
            tq->p_amount=(q->sizeKB/page_len)+1;
        sHead = (pt*)malloc(sizeof(pt));      //����ռ�

        tq->pHead=sHead;                //����������׵�ַ����������ָ����������ַ
        tq->pHead->next=NULL;           //ĩβ��ע
        tq->next = tp->next;            //�����������ַ
        tp->next = tq;
        tp = tq;                        //������ָ����������ַָ�����
        q=q->next;                      //����ָ�����
    }
    print_table_one(T_head);            //��ӡ��������Ϣ
    return T_head;
}

// 8. ��ӡ������
void print_table_one(PTS *T_head)
{
    PTS *p;
    p=T_head->next;
    printf("\nÿҳ2kb����\n������  ����ҳ��\n");
    while(p!=NULL)
    {
        printf("  %d         %d\n",p->process,p->p_amount);
        page+=p->p_amount;//����ҳ��
        p=p->next;
    }
}



// ��������
//11. �贫��ҳ�ţ�������ڵ��ַ
pt *create_page(int amount,PTS *tp,int i,int j,int g[1][N]) //tp��ʾ�������ϵ�һ�����
{
    pt *p,*q;     //������ָ��
    int count2;    //��ǰҳ�Ƿ��ڹ�������
    p=tp->pHead;   //�Ӹ���������������׵�ַ
    q=p->next;     //ָ�����

    int len;       //����ҳ��
    // int time;
    int block;     //���
    len=tp->p_amount;   //����ҳ��
    //ҳ�����Ѿ����ڸ�ҳ�棬ֻ��Ҫ�޸�ʱ��
    count2 = isIn(tp,amount);

    q=p->next;

    //ҳ�治�ڵ�ǰ��ҳ���д���
    while(len>0 && count2==0)
    {
        if(p->next==NULL)
        {
            block=j;
            g[i][j]=1;
            empty_block--;   //�տ���������
            q=(pt*)malloc(sizeof(pt));   //����ռ�
            q->p_number=amount;    //�޸Ľ�����Ϣ
            q->p_block=block;
            Ltimes++;              //ϵͳʱ��++
            q->LRUtime=Ltimes;     //�޸ı�����ʱ��
            q->next = p->next;     //�����̼�������б�
            p->next = q;
            p = q;
            break;
        }
        else
        {
            p=p->next;   //ָ�����
            len--;       //����ҳ��--��װ����ҳ
        }
    }

    return tp->pHead;
}
// 12. �ж�ҳ���Ƿ����
int isIn(PTS *tp,int amount)          //�贫�븸�����ַ��ҳ��
{
    pt *p,*q;  //��������������ָ�룬
    int count2=0;

    p=tp->pHead;     //ָ��Ӹ�����ָ��������ͷ
    q=p->next;       //qָ��Ϊpָ�����һλ
    while(q!=NULL)    //�������������������
    {

        if(q->p_number==amount)             //����ҳ��
        {
            count2++;                       //����װ���־
            Ltimes++;                       //����ʱ��++
            q->LRUtime=Ltimes;              //�����������ʱ��
            printf("��%dҳ�Ѿ�װ��\n",amount);
            printf("�޸��������ʱ��Ϊ��%d\n",Ltimes);
            break;
        }
        else
            q=q->next;            //ָ�����
    }
    return count2;        //����1Ϊ�Ѿ�װ�룬����0Ϊû��װ��
}

// 9. ��ʼ��ҳ��
void init_pageTable(pcb *head,PTS *T_head,int num,int g[1][N])
{
    PTS *p,*q;
    int process_number;//��������
    int amount;//ҳ��
    int i,j;
    p = T_head;
    //���̺�ֻ��������ɲ���ִ��ȫ���滻LRU�㷨
    process_number=Sort1(head); //�������Ҫִ�еĽ��̺�[1,num]
    printf("\n��ǰҪִ�н���%d\n",process_number);
    p=T_head;
    q=p->next;
    while(q!=NULL)         //���������Ϊ��
    {
        if(q->process==process_number)    //�����ǰ������ָ����ǵ�ǰҪִ�еĽ���
        {
            amount=rand()%q->p_amount;//[0,p_amount-1] ��������ý����е�һҳ����ִ�С�
            printf("��ǰִ��ҳ%d\n",amount);

            if(empty_block>0)         //�����ǰ�տ飨ҳ������ʣ��
            {

                for(int c = 2; c < N; c++) //��2��ʼ���ҿտ飬0,1�Ѿ���OS�ں�ռ��
                {
                    i = 0;
                    j = c;    //����
                    if(g[0][j] == 0)
                    {
                        create_page(amount,q,i,j,g);
                        break;
                    }
                }
            }

            else //û�пտ飬ִ���û��㷨
            {
                int count1;
                count1 = isIn(q,amount);//�жϵ�ǰҳ�Ƿ��Ѿ���ҳ���У������������û�������0
                if(count1 == 0)
                {
                    printf("\n����������������������ʱ��ִ��LRU����ҳ���滻������������������\n");
                    LRU(g,T_head);
                    printf("������������������������ҳ���滻��ɡ�����������������������\n");
                    printf("\n");


                    for(int c = 2; c < N; c++)    //���滻����Ľ���װ��
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

//10. �Խ��̵����ȼ���������
int Sort1(pcb *head)    //����Ϊͷָ�룬��ֵʱ����ͷָ��ĵ�ַ
{
    pcb *q, *p, *tail, *temp;
    tail = NULL;   //�������β����ΪNULL
    q = head;      //�������ͷָ���ַ��ֵ��q���Ӷ����в���

    while((head->next) != tail)     //�����û�ܵ������β���������ִ��
    {
        p = head->next;
        q = head;
        while(p->next != tail)          //����û�ܵ�β��������ִ��
        {
            //�����ȼ��͵Ľ��̷ŵ������β��
            if((p->priority) > (p->next->priority))    //���p�����ȼ�û����һ���ڵ�����ȼ��ߣ����ȼ�������ԽС�����ȼ�Խ�ߣ�
            {
                q->next = p->next;
                temp = p->next->next;
                p->next->next = p;
                p->next = temp;
                p = q->next;
            }
            p = p->next;     //ָ����ƣ������Ƚ�
            q = q->next;
        }
        tail = p;
    }
    //�൱��һ��ð������
    head->next->priority+=1;
    return head->next->name;
}


// 13. LRU��̭�㷨
void LRU(int g[1][N],PTS *T_head) //ֻҪʹ�ù��ͷŵ���β��������̭����ǰ���
{
    PTS *p,*q,*r;
    pt *pp,*qq;
    pt *pp1,*qq1;
    p=T_head;
    q=p->next;
    r=q;
    int shortime=100;
    int process;
    while(q != NULL)    //�ҳ������ϴη��ʼ����õĽ���
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
    printf("��ǰ���û������� %d�е�\n",process);    //���б���ǰ��Ľ����û���ȥ��
    while(r!=NULL)
    {
        if(r->process==process)     //�����ǰָ��ָ��Ľ�����Ҫ���û��Ľ���������û�������ָ�����
        {
            pp1=r->pHead;   //ҳ���׵�ַ
            qq1=pp1->next;

            while(qq1!=NULL)    //ҳ������Ϊ��
            {

                if(qq1->LRUtime==shortime)
                {

                    int p_block;
                    int i,j;
                    p_block=qq1->p_block;
                    printf("          ҳ�� %d,��� %d   \n",qq1->p_number,qq1->p_block);
                    i=0;
                    j=p_block;

                    Ltimes++;
                    qq1->LRUtime = Ltimes;
                    g[i][j]=0;
                    empty_block++;
                    printf("\n****LRUλʾͼ****\n");
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


// 14. ��ӡ
void print_table2(PTS *T_head)
{
    PTS *p;
    pt *pp;
    p=T_head->next;

    while(p!=NULL)
    {
        printf("\n������  ����ҳ��  \n");

        printf(" %d         %d\n",p->process,p->p_amount);
        printf("ҳ��  ���  ���ʹ��ʱ��\n");
        pp=p->pHead->next;
        while(pp!=NULL)
        {

            printf(" %2d     %2d      %2d\n",pp->p_number,pp->p_block,pp->LRUtime);
            pp=pp->next;
        }
        p=p->next;
    }
}

// 15. ���ַ
int serve(PTS *T_head)          //�贫�븸�����ַ��ҳ��
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
            b[nn1][0]=pp->p_number;//�洢ҳ��
            b[nn1][1]=pp->p_block;//���
            b[nn1][2]=pp->LRUtime;
            pp=pp->next;
            nn1++;
        }
        p=p->next;
    }

    printf("ҳ��  ���  \n");
    for(int i=0; i<nn1; i++)
    {
        printf(" %2d     %2d\n",b[i][0],b[i][1]);

    }
}
//16. ��ҳ�ţ��߼���ַ�����ţ������ַ��
int change(int h)
{
    for(int i=0; i<50; i++)
    {
        if(b[i][0]==h)
            return b[i][1];//���ؿ��
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
    printf("             -------------------------------�������ҳ�洢����ϵͳ------------------------------\n");
    printf("             -------------------------------************************--------------------------\n");
    head = Init();//��ʼ����ͷ
    while(flag!=1)
    {
        flag=1;
        printf("\n �����������Ŀ:");
        scanf("%d",&num);
        if(num > 10)          //���̸���������󲢷��ȣ�����Ϊ10�������û�
        {
            printf("���������Ϊ10�����������������Ŀ��");
            flag=0;
        }
    }

    output(head,num);     //�������ɵĽ���
    Ltimes=0;
    init_weishi(G);       //��ʼ��λʾͼ
    thead=Init2();
    create_table(head,thead);   //�����������������ơ�����ҳ�������ݼ���ó��������̶�Ӧ��ҳ���ַ��

    printf("\nҪ����ҳ�ĸ���Ϊ��%d\n",page);   //���Ҫ����Ľ���ҳ��֮�ͣ�pageΪȫ�ֱ�����
    printf("--------------------------------------------------------\n\n\n");
    for(i=0; i<page; i++)
    {
        init_pageTable(head,thead,num,G);    //��ҳ
        print_weish(G);              //��ӡλʾͼ�۲�ִ�����
        print_table2(thead);     //��ӡ���̱���ҳ�����������ĵ�ҳÿ���������������ҳ���Դ�ӡ�������ҳ���ậ�����е�ҳ��
        printf("--------------------------------------------------------\n\n\n");
    }

    serve(thead);// ����Ӧ��ϵ����������


    int num2;
    printf("\n\n-------------�߼���ַ�������ַ----------\n");
    printf("����һ��ʮ�����߼���ַ(��3208):\n");
    scanf("%d",&num2);
    //10����ת2����
    for(int i=0; i<32; i++) //��ʼ���߼���ַ����
    {
        a[i]=0;
    }
    for(int i=0; i<24; i++) //��ʼ���߼���ַ����
    {
        a2[i]=0;
    }
    i=31;
    int k=23;
    int count3=0;
    while (num2>0)   //ѭ�����̵�ʼ�գ��ж�num2�Ƿ��ܱ�2����
    {
        a[i] = num2%2;   //������洢ÿ�γ���2֮������������϶���ż�ԣ���Ӧ������λ����ֵ
        if(i>=21)    a2[k]=a[i];//��11λҳ�ڵ�ַ��ͬ
        i --;
        k--;
        num2 = num2/2;
        count3++;
    }
    printf("\n\n�߼���ַʮ����ת��Ϊ����������:\n");
    for(int j=0; j < 32; j++)
    {
        printf("%d",a[j]);
        if((j+1)%4==0) printf(" ");

    }
    printf("\n\n��Ӧ11λҳ�ڵ�ַ����������:\n");
    for(int j=21; j < 32; j++)
    {
        printf("%d",a[j]);
        if((j+1)%4==0) printf(" ");

    }

    //printf("�߼���ַ��%dλ\n",count3);

    int pa;//��¼ҳ��
    pa=a[17]*8+a[18]*4+a[19]*2+a[20];
    printf("\n���Ӧҳ��Ϊ��%d\n",pa);

    int h = change(pa);//��¼���
    if(h!=0) //����ѯ����Ӧ�����ַ�������
    {
        int i2=12;//�������±�23-11=12��ʼ
        printf("\n\n��Ӧ���Ϊ:%d\n",h);
        while (h>0)   //ѭ�����̵�ʼ�գ��ж�num2�Ƿ��ܱ�2����
        {
            a2[i2] = h%2;   //������洢ÿ�γ���2֮������������϶���ż�ԣ���Ӧ������λ����ֵ
            h = h/2;
            i2--;
        }

        printf("\n\n��Ӧ�����ַ����������:\n");
        for(int j=0; j < 24; j++)
        {
            printf("%d",a2[j]);
            if((j+1)%4==0) printf(" ");

        }
    }
    else
        printf("\nδ���ҵ���Ӧ�����ַ��");
    printf("\n");
    return 0;
}

