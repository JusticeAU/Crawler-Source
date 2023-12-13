#include "BillboardList.h"

BillboardList::BillboardList()
{
    glGenVertexArrays(1, &buffer);
}

BillboardList::~BillboardList()
{
    glDeleteVertexArrays(1, &buffer);
}

void BillboardList::Render()
{
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * positions.size(), positions.data(), GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), 0); // position

    glDrawArrays(GL_POINTS, 0, positions.size());

    glDisableVertexAttribArray(0);
}
