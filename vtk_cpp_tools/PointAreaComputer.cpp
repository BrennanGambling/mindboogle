/* ********************************************************************
 * PointAreaComputer
 *
 * Copyright 2012 Joachim Giard
 * Universit√© catholique de Louvain, Belgium
 * Apache License, Version 2.0
 *
 * Computes the voronoi area of each point
 *
 * Arno Klein added lines to avoid NaN errors in 2017.
 *
 * *******************************************************************/


#include "PointAreaComputer.h"

#include <vtkCellArray.h>
#include <vtkDoubleArray.h>
#include <vtkMath.h>
#include <vtkPolyDataWriter.h>
#include <vtkPointData.h>

PointAreaComputer::PointAreaComputer(vtkPolyData *mesh)
{
    m_mesh = mesh;
    m_pointsArea = vtkDoubleArray::New();
}

PointAreaComputer::~PointAreaComputer()
{
    m_pointsArea->Delete();
}

void PointAreaComputer::ComputeArea()
{
    m_pointsArea = ComputeVoronoiArea();
}

vtkDoubleArray* PointAreaComputer::GetArea()
{
    return m_pointsArea;
}

void PointAreaComputer::WriteIntoFile(char *fileName)
{
    vtkPolyDataWriter* writer=vtkPolyDataWriter::New();
    writer->SetFileName(fileName);
    m_mesh->GetPointData()->SetScalars(m_pointsArea);
    writer->SetInputData(m_mesh);
    writer->Write();
    writer->Delete();
}


vtkDoubleArray* PointAreaComputer::ComputeVoronoiArea()
{
    // initialisation
    vtkCellArray* cells=m_mesh->GetPolys();
    int nbPolys = cells->GetNumberOfCells();
    vtkDoubleArray* pointsArea = vtkDoubleArray::New();

    for(int i=0;i<m_mesh->GetNumberOfPoints();i++)
    {
        pointsArea->InsertNextValue(0);
    }

    int cellIds[3];

    double pos1[3];
    double pos2[3];
    double pos3[3];

    float a;
    float b;
    float c;

    float alpha;
    float beta;
    float gamma;

    float area, areaA, areaB, areaC;

    bool obtuse = false;

    for (int i=0;i<nbPolys;i++)
    {
        cellIds[0]=m_mesh->GetCell(i)->GetPointId(0);
        cellIds[1]=m_mesh->GetCell(i)->GetPointId(1);
        cellIds[2]=m_mesh->GetCell(i)->GetPointId(2);

        if(cellIds[0]>m_mesh->GetNumberOfPoints()||cellIds[1]>m_mesh->GetNumberOfPoints()||cellIds[2]>m_mesh->GetNumberOfPoints())
        {
            cout<<"point surface cell id problem: "<<i<<" "<<cellIds[0]<<" "<<cellIds[1]<<" "<<cellIds[2]<<endl;
        }
        else
        {
            m_mesh->GetPoint(cellIds[0],pos1);
            m_mesh->GetPoint(cellIds[1],pos2);
            m_mesh->GetPoint(cellIds[2],pos3);

            // distances between points of each triangle
            a=sqrt(vtkMath::Distance2BetweenPoints(pos3,pos2));
            b=sqrt(vtkMath::Distance2BetweenPoints(pos1,pos3));
            c=sqrt(vtkMath::Distance2BetweenPoints(pos2,pos1));

            // set 1/3 areas to zero by default
            areaA = 0;
            areaB = 0;
            areaC = 0;

            // if distances greater than zero
            if(a>0 && b>0 && c>0)
            {
                // angles
                alpha = acos( (pow(b,2) + pow(c,2) - pow(a,2)) / (2*b*c) );
                beta = acos( (pow(a,2) + pow(c,2) - pow(b,2)) / (2*a*c) );
                gamma = acos( (pow(b,2) + pow(a,2) - pow(c,2)) / (2*b*a) );

                // don't set 1/3 areas if angles are not numbers
                if( vtkMath::IsNan(alpha) || vtkMath::IsNan(beta) || vtkMath::IsNan(gamma) )
                {
                }
                // else set 1/3 areas
                else
                {
                    if( alpha > vtkMath::Pi()/2 || beta > vtkMath::Pi()/2 || gamma > vtkMath::Pi()/2 )
                    {
                        // Herons's formula
                        area = 0.25 * sqrt( (a+b+c)*(b+c-a)*(a-b+c)*(a+b-c) );

                        if(alpha > vtkMath::Pi()/2)
                        {
                            areaA = area/2;
                            areaB = area/4;
                            areaC = area/4;
                        }
                        else if (beta > vtkMath::Pi()/2)
                        {
                            areaA = area/4;
                            areaB = area/2;
                            areaC = area/4;
                        }
                        else if (gamma > vtkMath::Pi()/2)
                        {
                            areaA = area/4;
                            areaB = area/4;
                            areaC = area/2;
                        }
                    }
                    else
                    {
                        areaA = (pow(b,2) / tan(beta) + pow(c,2) / tan(gamma)) /8;
                        areaB = (pow(a,2) / tan(alpha) + pow(c,2) / tan(gamma)) /8;
                        areaC = (pow(b,2) / tan(beta) + pow(a,2) / tan(alpha)) /8;
                    }
                }
            }

            // add 1/3 of the face area to each point of the triangle
            pointsArea->SetValue(cellIds[0],pointsArea->GetValue(cellIds[0])+ areaA);
            pointsArea->SetValue(cellIds[1],pointsArea->GetValue(cellIds[1])+ areaB);
            pointsArea->SetValue(cellIds[2],pointsArea->GetValue(cellIds[2])+ areaC);
        }
    }

    return pointsArea;

}


