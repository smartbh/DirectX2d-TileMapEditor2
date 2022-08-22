#include "framework.h"

ObTileMap::ObTileMap()
{
    SafeRelease(vertexBuffer);
    SafeDeleteArray(vertices);
    for (int i = 0; i < 4; i++)
        SafeDelete(tileImages[i]);

    tileSize.x = 20;
    tileSize.y = 20;

    file = "map1.txt";
    scale = Vector2(50.0f, 50.0f);

    tileImages[0] = new ObImage(L"Tile.png");
    tileImages[0]->maxFrame = Int2(8, 6);
    tileImages[1] = new ObImage(L"Tile2.png");
    tileImages[1]->maxFrame = Int2(11, 7);
    tileImages[2] = new ObImage(L"pepe.png");
    tileImages[3] = nullptr;

    ResizeTile(tileSize);

}

ObTileMap::~ObTileMap()
{
    SafeRelease(vertexBuffer);
    SafeDeleteArray(vertices);

    for (int i = 0; i < 4; i++)
        SafeDelete(tileImages[i]);
}

void ObTileMap::Render()
{
    if (!visible)return;

    GameObject::Render();

    for (int i = 0; i < 4; i++)
    {
        if (tileImages[i])
        {
            D3D->GetDC()->PSSetShaderResources(i, 1, &tileImages[i]->SRV);
            D3D->GetDC()->PSSetSamplers(i, 1, &tileImages[i]->sampler);
        }
    }

    tileMapShader->Set();

    UINT stride = sizeof(VertexTile);
    UINT offset = 0;

    //버텍스버퍼 바인딩
    D3D->GetDC()->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);
    D3D->GetDC()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    D3D->GetDC()->Draw(tileSize.x * tileSize.y * 6, 0);
}

bool ObTileMap::WorldPosToTileIdx(Vector2 WPos, Int2& TileIdx)
{
    WPos -= GetWorldPos();

    Vector2 tileCoord;

    //타일 맵좌표로 변환 식
    tileCoord.x = WPos.x / scale.x;
    tileCoord.y = WPos.y / scale.y;

    if ((tileCoord.x < 0) or (tileCoord.y < 0) or
        (tileCoord.x >= tileSize.x) or (tileCoord.y >= tileSize.y))
    {
        return false;
    }

    TileIdx.x = tileCoord.x;
    TileIdx.y = tileCoord.y;

    return true;

}

void ObTileMap::SetTile(Int2 TileIdx, Int2 FrameIdx, int ImgIdx, int TileState, Color color)
{
    int tileIdx = tileSize.x * TileIdx.y + TileIdx.x;

    vertices[tileIdx * 6 + 0].uv.x = FrameIdx.x / (float)tileImages[ImgIdx]->maxFrame.x;
    vertices[tileIdx * 6 + 1].uv.x = FrameIdx.x / (float)tileImages[ImgIdx]->maxFrame.x;
    vertices[tileIdx * 6 + 5].uv.x = FrameIdx.x / (float)tileImages[ImgIdx]->maxFrame.x;

    vertices[tileIdx * 6 + 2].uv.x = (FrameIdx.x + 1.0f) / (float)tileImages[ImgIdx]->maxFrame.x;
    vertices[tileIdx * 6 + 3].uv.x = (FrameIdx.x + 1.0f) / (float)tileImages[ImgIdx]->maxFrame.x;
    vertices[tileIdx * 6 + 4].uv.x = (FrameIdx.x + 1.0f) / (float)tileImages[ImgIdx]->maxFrame.x;

    vertices[tileIdx * 6 + 3].uv.y = FrameIdx.y / (float)tileImages[ImgIdx]->maxFrame.y;
    vertices[tileIdx * 6 + 1].uv.y = FrameIdx.y / (float)tileImages[ImgIdx]->maxFrame.y;
    vertices[tileIdx * 6 + 5].uv.y = FrameIdx.y / (float)tileImages[ImgIdx]->maxFrame.y;

    vertices[tileIdx * 6 + 2].uv.y = (FrameIdx.y + 1.0f) / (float)tileImages[ImgIdx]->maxFrame.y;
    vertices[tileIdx * 6 + 0].uv.y = (FrameIdx.y + 1.0f) / (float)tileImages[ImgIdx]->maxFrame.y;
    vertices[tileIdx * 6 + 4].uv.y = (FrameIdx.y + 1.0f) / (float)tileImages[ImgIdx]->maxFrame.y;

    for (int i = 0; i < 6; i++)
    {
        vertices[tileIdx * 6 + i].tileMapIdx = ImgIdx;
        vertices[tileIdx * 6 + i].color = color;
        vertices[tileIdx * 6 + i].tileState = TileState;
    }
    
    //갱신
    D3D->GetDC()->UpdateSubresource(vertexBuffer, 0, NULL, vertices, 0, 0);
}

void ObTileMap::RenderGui(Int2& GuiPickingIdx, int& ImgIdx)
{
    if (ImGui::InputInt("ImgIdx", &ImgIdx))
    {
        if (ImgIdx > 2)
        {
            ImgIdx = 0; //첫번째이미지로 돌리기
        }
        else if (ImgIdx < 0)
        {
            //마지막 이미지로 돌리기
            ImgIdx = 2;
        }
    }

    Int2 MF = tileImages[ImgIdx]->maxFrame;
    ImVec2 size;

    size.x = 300.0f / (float)MF.x;
    size.y = 300.0f / (float)MF.y;

    //텍스쳐 좌표
    ImVec2 LT, RB;
    int index = 0;
    for (UINT i = 0; i < MF.y; i++)
    {
        for (UINT j = 0; j < MF.x; j++)
        {
            if (j != 0)
            {
                //같은줄에 배치
                ImGui::SameLine();
            }
            //텍스쳐 좌표
            LT.x = 1.0f / MF.x * j;
            LT.y = 1.0f / MF.y * i;
            RB.x = 1.0f / MF.x * (j + 1);
            RB.y = 1.0f / MF.y * (i + 1);

            ImGui::PushID(index);
            if (ImGui::ImageButton((void*)tileImages[ImgIdx]->SRV, size, LT, RB))
            {
                GuiPickingIdx.x = j;
                GuiPickingIdx.y = i;
            }
            index++;
            ImGui::PopID();
        }
    }
}

void ObTileMap::ResizeTile(Int2 TileSize)
{
    VertexTile* Vertices = new VertexTile[TileSize.x * TileSize.y * 6];

    //세로
    for (int i = 0; i < TileSize.y; i++)
    {
        //가로
        for (int j = 0; j < TileSize.x; j++)
        {
            int tileIdx = TileSize.x * i + j;

            //0
            Vertices[tileIdx * 6].position.x = 0.0f + j;
            Vertices[tileIdx * 6].position.y = 0.0f + i;
            Vertices[tileIdx * 6].uv = Vector2(0.0f, 1.0f);
            //1                             
            Vertices[tileIdx * 6 + 1].position.x = 0.0f + j;
            Vertices[tileIdx * 6 + 1].position.y = 1.0f + i;
            Vertices[tileIdx * 6 + 1].uv = Vector2(0.0f, 0.0f);
            //2                             
            Vertices[tileIdx * 6 + 2].position.x = 1.0f + j;
            Vertices[tileIdx * 6 + 2].position.y = 0.0f + i;
            Vertices[tileIdx * 6 + 2].uv = Vector2(1.0f, 1.0f);
            //3
            Vertices[tileIdx * 6 + 3].position.x = 1.0f + j;
            Vertices[tileIdx * 6 + 3].position.y = 1.0f + i;
            Vertices[tileIdx * 6 + 3].uv = Vector2(1.0f, 0.0f);
            //4
            Vertices[tileIdx * 6 + 4].position.x = 1.0f + j;
            Vertices[tileIdx * 6 + 4].position.y = 0.0f + i;
            Vertices[tileIdx * 6 + 4].uv = Vector2(1.0f, 1.0f);
            //5
            Vertices[tileIdx * 6 + 5].position.x = 0.0f + j;
            Vertices[tileIdx * 6 + 5].position.y = 1.0f + i;
            Vertices[tileIdx * 6 + 5].uv = Vector2(0.0f, 0.0f);
        }
    }

    //Copy
    if (vertices)
    {
        Int2 Min = Int2(min(TileSize.x, tileSize.x), min(TileSize.y, tileSize.y));

        for (int i = 0; i < Min.y; i++)
        {
            for (int j = 0; j < Min.x; j++)
            {
                int SrcIdx = tileSize.x * i + j;
                int DestIdx = TileSize.x * i + j;
                for (int k = 0; k < 6; k++)
                {
                    Vertices[DestIdx * 6 + k] = vertices[SrcIdx * 6 + k];
                }
            }
        }
    }

    SafeDeleteArray(vertices);
    vertices = Vertices;
    tileSize = TileSize;
    SafeRelease(vertexBuffer);

    //CreateVertexBuffer
    {
        D3D11_BUFFER_DESC desc;
        desc = { 0 };
        desc.Usage = D3D11_USAGE_DEFAULT;
        desc.ByteWidth = sizeof(VertexTile) * tileSize.x * tileSize.y * 6;
        desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        D3D11_SUBRESOURCE_DATA data = { 0 };
        data.pSysMem = vertices;
        HRESULT hr = D3D->GetDevice()->CreateBuffer(&desc, &data, &vertexBuffer);
        Check(hr);
    }
}

void ObTileMap::Save()
{
    ofstream fout;
    string path = "../Contents/TileMap/" + file;
    //ios::out 쓰기용으로 열겟다
    fout.open(path.c_str(), ios::out);

    if (fout.is_open())
    {
        for (int i = 0; i < 4; i++)
        {
            fout << i << " ";

            if (tileImages[i])
            {
                string imgFile = "";
                imgFile.assign(tileImages[i]->file.begin(), tileImages[i]->file.end());

                fout << imgFile << " ";
                fout << tileImages[i]->maxFrame.x << " " << tileImages[i]->maxFrame.y << endl;
            }
            else
            {
                fout << "N" << endl;
            }
        }

        fout << "TileSize " << tileSize.x << " " << tileSize.y << endl;
        fout << "TileScale " << scale.x << " " << scale.y << endl;
        fout << "TilePosition " << GetWorldPos().x << " " << GetWorldPos().y << endl;

        //세로
        for (int i = 0; i < tileSize.y; i++)
        {
            //가로
            for (int j = 0; j < tileSize.x; j++)
            {
                int tileIdx = tileSize.x * i + j;

                fout << j << " " << i << " "
                    << vertices[tileIdx * 6 + 1].uv.x << " " << vertices[tileIdx * 6 + 1].uv.y << " "
                    << vertices[tileIdx * 6 + 2].uv.x << " " << vertices[tileIdx * 6 + 2].uv.y << " "
                    << vertices[tileIdx * 6].color.x << " " << vertices[tileIdx * 6].color.y << " "
                    << vertices[tileIdx * 6].color.z << " " << vertices[tileIdx * 6].color.w << " "
                    << vertices[tileIdx * 6].tileMapIdx << " " << vertices[tileIdx * 6].tileState << endl;
            }
        }

        fout.close();
    }

}

void ObTileMap::Load()
{
    ifstream fin;
    string path = "../Contents/TileMap/" + file;
    fin.open(path.c_str(), ios::in);
    string temp;

    if (fin.is_open())
    {
        for (int i = 0; i < 4; i++)
        {
            SafeDelete(tileImages[i]);

            int idx; string Imgfile;
            fin >> idx;
            fin >> Imgfile;

            if (Imgfile != "N")
            {
                wstring wImgFile = L"";
                wImgFile.assign(Imgfile.begin(), Imgfile.end());
                tileImages[i] = new ObImage(wImgFile);
                fin >> tileImages[i]->maxFrame.x >> tileImages[i]->maxFrame.y;
            }
        }

        Int2 TileSize;
        fin >> temp >> TileSize.x >> TileSize.y;

        ResizeTile(TileSize);

        fin >> temp >> scale.x >> scale.y;

        Vector2 pos;
        fin >> temp >> pos.x >> pos.y;
        SetWorldPos(pos);

        for (int i = 0; i < tileSize.y; i++)
        {
            //가로
            for (int j = 0; j < tileSize.x; j++)
            {
                int temp; Vector2 MinUV, MaxUV; Color color; float tileMapIdx, tileMapState;

                fin >> temp >> temp >> MinUV.x >> MinUV.y >> MaxUV.x >> MaxUV.y
                    >> color.x >> color.y >> color.z >> color.w >> tileMapIdx >> tileMapState;

                int tileIdx = tileSize.x * i + j;

                vertices[tileIdx * 6].uv = Vector2(MinUV.x, MaxUV.y);
                vertices[tileIdx * 6 + 1].uv = Vector2(MinUV.x, MinUV.y);
                vertices[tileIdx * 6 + 2].uv = Vector2(MaxUV.x, MaxUV.y);
                vertices[tileIdx * 6 + 3].uv = Vector2(MaxUV.x, MinUV.y);
                vertices[tileIdx * 6 + 4].uv = Vector2(MaxUV.x, MaxUV.y);
                vertices[tileIdx * 6 + 5].uv = Vector2(MinUV.x, MinUV.y);

                for (int k = 0; k < 6; k++)
                {
                    vertices[tileIdx * 6 + k].color = color;
                    vertices[tileIdx * 6 + k].tileMapIdx = tileMapIdx;
                    vertices[tileIdx * 6 + k].tileState = tileMapState;
                }
            }
        }

        D3D->GetDC()->UpdateSubresource(vertexBuffer, 0, NULL, vertices, 0, 0);

        fin.close();
    }
}


