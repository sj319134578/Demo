import java.io.*;
import java.util.ArrayList;

/**

 */
public class max_bag {
    static int n;           // 描述物品个数
    static int c;           // 描述背包容量
    static int[] v;     // 描述物品价值
    static int[] w;    // 描述物品重量


    public static void main(String[] args) throws IOException {
        ArrayList<String> arrayList = new ArrayList<>();
        try {
            //读取文件
            File file = new File("d:/testdata/beibao7.in");
            InputStreamReader input = new InputStreamReader(new FileInputStream(file));
            BufferedReader bf = new BufferedReader(input);
            // 按行读取字符串
            String str;
            while ((str = bf.readLine()) != null) {
                arrayList.add(str);
            }
            bf.close();
            input.close();
        } catch (IOException e) {
            e.printStackTrace();
        }
        // 对ArrayList中存储的字符串进行处理
        int length = arrayList.size();
        int width = arrayList.get(0).split(" ").length;
        int array[][] = new int[length][width];
        for (int i = 0; i < length; i++) {
            for (int j = 0; j < width; j++) {
                String s = arrayList.get(i).split(" ")[j];
                array[i][j] = Integer.parseInt(s);
            }
        }
        //输出数组中的字符串
        for (int i = 0; i < length; i++) {
            for (int j = 0; j < width; j++) {
                System.out.print(array[i][j] + " ");
            }
            System.out.println();
        }

        // 初始赋值操作
        v = new int[length];
        for (int i = 1; i < length; i++) {
              v[i]=array[i][0];     //价值赋值
            //System.out.println(v[i]+" ");
        }
        w = new int[length];
        for (int i = 1; i < length; i++) {
            w[i]=array[i][1];       //重量赋值
            //System.out.println(w[i]+" ");
        }
        c = array[0][0];        //存储背包容量
        n = array[0][1];        //存储物品数量
        int [][] m = new int[v.length][c+1];//定义m二维数组用来表示所有的价值,m[i][j]表示第i物品装入容量为j的背包的最大值
        int [] x = new int[w.length];    //解空间集
        System.out.println("贪心算法：");
        Knapsack(v,w,c,m);
        System.out.println("m[0][c]="+m[0][c]);
        for(int i=0;i<x.length;i++){
            System.out.print(x[i]+" ");
        }
        System.out.println();
        for(int i=0;i<v.length;i++){
            for(int j=0;j<c+1;j++){
                System.out.print(m[i][j]+"\t");
            }
            System.out.println();
        }
        System.out.println("动态规划算法：");
        traceback(m,w,c,x);

        System.out.println("m[0][c]="+m[0][c]);
        for(int i=0;i<x.length;i++){
            System.out.print(x[i]+" ");
        }
        System.out.println();
        for(int i=0;i<v.length;i++){
            for(int j=0;j<c+1;j++){
               // System.out.print(m[i][j]+"\t");
            }
            System.out.println();
        }

    }
    public static void traceback(int [][]m,int []w,int c,int []x){
        int n = w.length-1;
        for(int i=0;i<n;i++)
            if(m[i][c]==m[i+1][c]) x[i] = 0;
            else{
                x[i] = 1;
                c-=w[i];
            }
        x[n] = (m[n][c]>0)?1:0;
    }

    public static void Knapsack(int v[],int w[],int c,int [][]m){
        int n = v.length-1;
        int jMax = Math.min(w[n]-1, c);
        for(int j=0;j<=jMax;j++) m[n][j] = 0;
        for(int j=w[n];j<=c;j++) m[n][j] = v[n];

        for(int i=n-1;i>=0;i--){
            jMax = Math.min(w[i]-1, c);
            for(int j=0;j<=jMax;j++)
                m[i][j] = m[i+1][j];
            for(int j=w[i];j<=c;j++)
                m[i][j] = Math.max(m[i+1][j],m[i+1][j-w[i]]+v[i]);
        }
    }


    }
