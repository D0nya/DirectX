#pragma once

template <class T>
class VertexCreator
{
private:
	int size;
	T* vertices;
public:
	VertexCreator(int size)
	{
		vertices = new T[size];
	}
	VertexCreator(const VertexCreator&) = delete;
	VertexCreator& operator=(const VertexCreator&) = delete;
	~VertexCreator()
	{
		delete vertices;
	}

	T GetVertices();
	int Size();
};