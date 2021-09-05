
#include "AOVSort.h"



GraphAdjList::GraphAdjList()
{
	numVertexes = 0 ;
	numEdges = 0;
}
GraphAdjList::~GraphAdjList()
{
	for (size_t i = 0 ; i < adjList.size(); i++)
	{
		delete adjList[i];
	}
	adjList.clear();
}
void GraphAdjList::AddVertexNode(VertextNode* vtNode)
{
	adjList.push_back(vtNode);
	numVertexes =  adjList.size();
}


VertextNode* GraphAdjList::GetVertexNode(const wstring& nodeName)
{
	for (size_t i = 0 ; i  < adjList.size(); i++)
	{
		if ( adjList[i]->NodeName == nodeName)
		{
			return adjList[i];
		}
	}
	return NULL;
}
int GraphAdjList::GetVertexNodeIndex(const wstring& nodeName)
{
	for (size_t i = 0 ; i  < adjList.size(); i++)
	{
		if ( adjList[i]->NodeName == nodeName)
		{
			return i;
		}
	}
	return -1;
}
void GraphAdjList::UpdateGLEdgeIndex()
{
	for (size_t i = 0 ; i < adjList.size(); i++)
	{
		for (size_t k = 0; k < adjList.size(); k++)
		{
			if (i == k)
			{
				continue;
			}
			vector<EdgeNode*>::iterator itEdgeNode = adjList[k]->m_edges.begin();
			vector<EdgeNode*>::iterator itEdgeEnd = adjList[k]->m_edges.end();
			for ( ; itEdgeNode !=  itEdgeEnd; itEdgeNode++ )
			{
				if ( (*itEdgeNode)->NodeName == adjList[i]->NodeName )
				{
					(*itEdgeNode)->adjvex = i;
					break;
				}
			}
		}
	}
}
void GraphAdjList::RemoveNode(wstring& nodeName)
{
	vector<VertextNode*>::iterator itNode = adjList.begin();
	vector<VertextNode*>::iterator itEnd = adjList.end();
	int iIndex = 0;
	for (;itNode != itEnd ; itNode++)
	{
		if ( (*itNode)->NodeName == nodeName)
		{
			for (size_t i = 0 ; i < adjList.size(); i++)//ɾ���͸ö����йص���Ϣ
			{
				vector<EdgeNode*>::iterator itEdgeNode = adjList[i]->m_edges.begin();
				vector<EdgeNode*>::iterator itEdgeEnd = adjList[i]->m_edges.end();
				for ( ; itEdgeNode !=  itEdgeEnd; itEdgeNode++ )
				{
					if ( (*itEdgeNode)->adjvex == iIndex )
					{
						delete *itEdgeNode;
						adjList[i]->m_edges.erase(itEdgeNode);
						break;
					}
				}
			}
			delete *itNode;
			adjList.erase(itNode);
			numVertexes = adjList.size();
			UpdateGLEdgeIndex();
			return ;
		}
		iIndex++;
	}
}



TopoligcSort TopologicalSort(GraphAdjList* GL,vector<wstring>& sortArray)
{
	EdgeNode* e;

	int k,gettop;

	int count = 0 ;//����ͳ�ƶ���ĸ���

	stack<int> stack;

	for (int i = 0 ; i < GL->numVertexes; i++)
	{
		if (GL->adjList[i]->in == 0)
		{
			stack.push(i);//�����Ϊ0 �Ķ�����ջ
		}
	}

	while( stack.size() )
	{
		gettop = stack.top();
		stack.pop();
		sortArray.push_back(GL->adjList[gettop]->NodeName);
		count++;

		int iLen = GL->adjList[gettop]->m_edges.size();
		if (iLen)
		{
			int indexEdges = 0;
			for ( ; indexEdges <  iLen; indexEdges++)
			{
				e = GL->adjList[gettop]->m_edges[indexEdges];
				k = e->adjvex;//�Դ˶��㻡�����
				if (! ( --(GL->adjList[k]->in) ) )//��k�Ŷ����ڽӵ����ȼ�1
				{
					stack.push(k);//��Ϊ0����ջ���Ա��´�ѭ�����
				}	
			}
		}
	}

	if (count < GL->numVertexes)//���count С�ڶ�������˵�����ڻ�
		return TopologicalSort_ERROR;
	else
		return TopologicalSort_OK;
}


TopoligcSort FindLoop(GraphAdjList* GL, vector<wstring>& sortArray,vector<wstring>& loopPackageArray)
{
	for ( size_t i = 0; i < sortArray.size(); i++ )
		GL->RemoveNode(sortArray[i]);

	if ( !GL->numVertexes )
		return NOLOOP;


	VertextNode* node =NULL;
	int index = 0;
	while( true )
	{
		node = GL->adjList[index];
		if (!node->m_edges.size())
			return NOLOOP;
		if (node->bVisit)
		{
			loopPackageArray.push_back(node->NodeName);
			break;
		}
		else
		{
			node->bVisit= true;
			index =  node->m_edges[0]->adjvex;
			loopPackageArray.push_back(node->NodeName);
		}
	}

    return HAVELOOP;
}
