#pragma  once

#include <stack>
#include <iostream>
#include <string>
#include <vector>
using namespace std;

class EdgeNode
{
public:
	EdgeNode(int index,wstring name = L"")
	{
		adjvex = index;
		NodeName = name;
	}
	~EdgeNode()
	{

	}
	int adjvex;//邻接点域，存储该顶点对应的下标
	wstring NodeName;//存储顶点的信息
};

class VertextNode
{

public:
	int in;//顶点入度
	int index;
	wstring NodeName;//存储顶点的信息
	vector<EdgeNode*> m_edges;//边表头指针
	bool bVisit;//这个成员在遍历回路时 作为标记使用。

public:
	VertextNode(wstring& strNodeName,int iIndex = -1)
	{
		in = 0;
		index =  iIndex;
		bVisit = false;
		NodeName = strNodeName;
	
	}
	~VertextNode()
	{
		for(size_t i = 0 ; i <  m_edges.size(); i++ )
		{
			delete m_edges[i];
		}

	}
	void AddEdgeNode(EdgeNode* pFirstEdge)
	{
		m_edges.push_back( pFirstEdge );
	}

	void IncreaseInNum(){ ++in; }
private:
	VertextNode(){};
};

class  GraphAdjList
{
public:

	vector<VertextNode*>  adjList;

	int numVertexes;//图中当前的顶点数

	int numEdges;//图中当前的边数

public:

	GraphAdjList();
	~GraphAdjList();

    void AddVertexNode(VertextNode* vtNode);
	VertextNode* GetVertexNode(const wstring& nodeName);
	
	int GetVertexNodeIndex(const wstring& nodeName);
	void UpdateGLEdgeIndex();
	void RemoveNode(wstring& nodeName);

};

enum TopoligcSort : unsigned short{
	TopologicalSort_ERROR                   = 0x0000,
	TopologicalSort_OK                       = 0x0001,
	HAVELOOP   = 0x0002,
	NOLOOP = 0x0003
};



TopoligcSort TopologicalSort(GraphAdjList* GL,vector<wstring>& sortArray);

TopoligcSort FindLoop(GraphAdjList* GL, vector<wstring>& sortArray,vector<wstring>& loopPackageArray);

