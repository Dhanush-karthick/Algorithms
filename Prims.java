//{ Driver Code Starts

import java.io.*;
import java.lang.*;
import java.util.*;

public class Prims {
    static BufferedReader br;
    static PrintWriter ot;

    public static void main(String args[]) throws IOException {
        br = new BufferedReader(new InputStreamReader(System.in));
        ot = new PrintWriter(System.out);
        int t = Integer.parseInt(br.readLine().trim());
        while (t-- > 0) {
            int V = Integer.parseInt(br.readLine().trim());
            int E = Integer.parseInt(br.readLine().trim());
            List<List<int[]>> list = new ArrayList<>();
            for (int i = 0; i < V; i++)
                list.add(new ArrayList<>());
            for (int i = 0; i < E; i++) {
                String[] s = br.readLine().trim().split(" ");
                int a = Integer.parseInt(s[0]);
                int b = Integer.parseInt(s[1]);
                int c = Integer.parseInt(s[2]);
                list.get(a).add(new int[] { b, c });
                list.get(b).add(new int[] { a, c });
            }
            ot.println(new Solution().spanningTree(V, E, list));

            ot.println("~");
        }
        ot.close();
    }
}
// } Driver Code Ends

// User function Template for Java

class Solution {
    static int spanningTree(int V, int E, List<List<int[]>> adj) {
        Queue<int[]> q = new PriorityQueue<>(Comparator.comparingInt(b -> b[2]));

        int sum = 0;
        ArrayList<int[]> edges = new ArrayList<>();
        boolean[] visited = new boolean[V];

        q.offer(new int[] { -1, 0, 0 });

        while (!q.isEmpty()) {
            int[] node = q.poll();
            int parent = node[0];
            int child = node[1];
            int cost = node[2];
            if (visited[child])
                continue;
            visited[child] = true;
            if (parent != -1) {
                edges.add(new int[] { parent, child });
                sum += cost;
            }

            for (int[] neighbor : adj.get(child)) {
                q.offer(new int[] { child, neighbor[0], neighbor[1] });
            }
        }

        return sum;
    }
}