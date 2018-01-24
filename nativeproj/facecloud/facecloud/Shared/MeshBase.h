#pragma once
#include "technique.h"
#include "ogldev_engine_common.h"
#include "ogldev_app.h"
#include "ogldev_camera.h"
#include "ogldev_util.h"
#include "ogldev_camera.h"
#include "ogldev_pipeline.h"

struct Vertex
{
	Vector3f m_pos;
	Vector2f m_tex;

	Vertex() {}

	Vertex(Vector3f pos, Vector2f tex)
	{
		m_pos = pos;
		m_tex = tex;
	}
};

class MeshBase
{
public:
	MeshBase();
	~MeshBase();

	void Init(Technique* ptechnique);
	void Render();

private:
	Technique* m_pTechnique;
	GLuint m_VBO;
	GLuint m_IBO;
};

