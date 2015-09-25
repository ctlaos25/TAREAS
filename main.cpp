#include<SDL2/SDL.h>
#include<SDL2/SDL_image.h>
#include<iostream>
#include<map>
#include<vector>
#include <stdlib.h>
#include<SDL2/SDL_mixer.h>
#include "TinyXml/tinyxml.h"
#include <SDL2/SDL_ttf.h>
#include <sstream>

using namespace std;

SDL_Window* window;
SDL_Renderer* renderer;
SDL_Event Event;
SDL_Texture *background, *texture_npc, *texture_tile, *Message, *imagen0;
SDL_Rect rect_background, rect_character, rect_npc, rect_tile, rect_tileset, rect_imagen0, Message_rect;//rectangulos de fondo, personaje, personaje no jugable, mapa,
TTF_Font *font;
SDL_Color Color;
int mapaa=0;
bool continuar = false;
//Warp: Portal de transferencia a otro mapa.
class Warp
{
public:
    int x;//pos en X
    int y;//pos en Y
    string mapa;//Nombre del mapa
    SDL_Rect rect;//Un rectángulo que representará el portal

    //constructor
    Warp(int x,int y,string mapa,SDL_Rect rect)
    {
        this->x=x;
        this->y=y;
        this->mapa=mapa;
        this->rect = rect;
    }
};

//función que devuelve boleano que identificará si el personaje hace colisión con un objeto no permitido, r1 representa al personaje
bool collision(SDL_Rect r1, SDL_Rect r2)
{
    if(r1.x+r1.w <= r2.x)
        return false;
    if(r1.x >= r2.x+r2.w)
        return false;
    if(r1.y+r1.h<=r2.y)
        return false;
    if(r1.y >= r2.y+r2.h)
        return false;
    return true;
}

//función que devuelve los portales presentes en un mapa, leído desde su xml.
vector<Warp*> getWarps(string archivo)
{
    TiXmlDocument doc(archivo.c_str());//cargamos el archivo xml
    doc.LoadFile();//comprobamos que este exista.
    //capas del mapa (nodos) del mapas en tinixml
    TiXmlElement *map_node = doc.FirstChild("map")->ToElement();// <tileset firstgid="1" name="Test" tilewidth="32" tileheight="32">
    //Cargando el primer warp (todos los war pertenecen a objectgrup)
    TiXmlNode*objectgroup_node = map_node->FirstChild("objectgroup");//<object name="warp" x="161" y="32" width="30" height="21">

    vector<Warp*> respuesta;//vector que retornaremos

    if(objectgroup_node==NULL)//verificamos la existencia de warps
        return respuesta;

    //Ciclo para meter los warps en un arreglo.
    for(TiXmlNode*object_node = objectgroup_node->FirstChild("object");//EL iterador es el primer hijo de objectgroup_node
        object_node!=NULL;//que el siguiente no sea nulo
        object_node=object_node->NextSibling("object"))//se pasa el siguiente warp
    {
        int x,y;
        string mapa;//nombre del mapa
        SDL_Rect rect;//creamos un rectangulo en SDL que representará el warp, le asignamos el mismo valor que tiene el wrap en el documento
        rect.x = atoi(object_node->ToElement()->Attribute("x"));
        rect.y = atoi(object_node->ToElement()->Attribute("y"));
        rect.w = atoi(object_node->ToElement()->Attribute("width"));
        rect.h = atoi(object_node->ToElement()->Attribute("height"));

        //Guardamos las propiedades del warp sig.
        TiXmlNode*properties = object_node->FirstChild("properties");
        for(TiXmlNode*property = properties->FirstChild("property");
            property!=NULL;
            property=property->NextSibling("property"))
        {
            if(strcmp(property->ToElement()->Attribute("name"),"x")==0)
            {

//                cout<<"X:"<<property->ToElement()->Attribute("value")<<endl;
                x=atoi(property->ToElement()->Attribute("value"));
            }
            if((property->ToElement()->Attribute("name"),"y")==0)
            {
                y=atoi(property->ToElement()->Attribute("value"));
            }
            if(strcmp(property->ToElement()->Attribute("name"),"mapa")==0)
            {
                mapa=property->ToElement()->Attribute("value");
            }
        }
        //Creamos el warp siguiente para anyadirlo a la lista
        Warp*warp = new Warp(x,y,mapa,rect);
//        cout<<warp->x<<endl;
//        cout<<warp->y<<endl;
//        cout<<warp->mapa<<endl;
//        cout<<warp->rect.x<<endl;
//        cout<<warp->rect.y<<endl;
//        cout<<warp->rect.w<<endl;
//        cout<<warp->rect.h<<endl;
        respuesta.push_back(warp);
    }

    return respuesta;
}

vector<int> getMapa(string archivo,int layer)
{
    vector<int> mapa;
    TiXmlDocument doc(archivo.c_str());
    bool loadOkay = doc.LoadFile();
    TiXmlElement *map_node = doc.FirstChild("map")->ToElement();
    TiXmlNode*layer_node_temp = map_node->FirstChild("layer");
    for(int i=1;i<layer;i++)
        layer_node_temp=layer_node_temp->NextSibling("layer");

    TiXmlElement *layer_node = layer_node_temp->ToElement();

    for(TiXmlNode *tile_node = layer_node->FirstChild("data")->FirstChild("tile");
        tile_node!=NULL;
        tile_node=tile_node->NextSibling("tile"))
    {
        mapa.push_back(atoi(tile_node->ToElement()->Attribute("gid")));
    }
    return mapa;
}

void showTTF(string str, int espacio)
{
    SDL_Surface* surfaceMessage = TTF_RenderText_Solid(font, str.c_str(), Color); // as TTF_RenderText_Solid could only be used on SDL_Surface then you have to create the surface first

    Message = SDL_CreateTextureFromSurface(renderer, surfaceMessage); //now you can convert it into a texture

    Message_rect.x = espacio;  //controls the rect's x coordinate
    Message_rect.y = 0; // controls the rect's y coordinte
    SDL_QueryTexture(Message, NULL, NULL, &Message_rect.w, &Message_rect.h);

}

void dibujarLayer(SDL_Renderer* renderer,vector<int>mapa)
{
    int x_pantalla = 0;
    int y_pantalla = 0;
    for(int i=0;i<mapa.size();i++)
    {
        int x = 0;
        int y = 0;
        for(int acum = 1;acum<mapa[i];acum++)
        {
            x+=32;
            if(acum%16==0)
            {
                y+=32;
                x=0;
            }
        }

    //            rect_tile.x = 32*(f[i]%16-1);
    //            rect_tile.y = 32*(mapa[i]/16);
        rect_tile.x = x;
        rect_tile.y = y;
        rect_tile.w = 32;
        rect_tile.h = 32;

        //cout<<rect_tile.x<<","<<rect_tile.y<<endl;

        rect_tileset.x = x_pantalla;
        rect_tileset.y = y_pantalla;

        if(mapa[i]!=0)
            SDL_RenderCopy(renderer, texture_tile, &rect_tile, &rect_tileset);

        x_pantalla+=32;
        if(x_pantalla>=320)
        {
            x_pantalla=0;
            y_pantalla+=32;
        }
    }
}

bool collisionLayer(vector<int>collision_map,SDL_Rect rect_personaje)
//    rect_personaje.x+=1;
//    rect_personaje.y+=1;
//    rect_personaje.w-=2;
//    rect_personaje.h-=2;
{

    int x_pantalla = 0;
    int y_pantalla = 0;
    for(int i=0;i<collision_map.size();i++)
    {
        int x = 0;
        int y = 0;
        for(int acum = 1;acum<collision_map[i];acum++)
        {
            x+=32;
            if(acum%16==0)
            {
                y+=32;
                x=0;
            }
        }

    //            rect_tile.x = 32*(mapa[i]%16-1);
    //            rect_tile.y = 32*(mapa[i]/16);
        rect_tile.x = x;
        rect_tile.y = y;
        rect_tile.w = 32;
        rect_tile.h = 32;

        //cout<<rect_tile.x<<","<<rect_tile.y<<endl;

        rect_tileset.x = x_pantalla;
        rect_tileset.y = y_pantalla;
        rect_tileset.w = 32;
        rect_tileset.h = 32;

        if(collision_map[i]!=0)
        {
            if(collision(rect_personaje,rect_tileset))
            {
                return true;
            }
        }

        x_pantalla+=32;
        if(x_pantalla>=320)
        {
            x_pantalla=0;
            y_pantalla+=32;
        }
    }
    return false;
}

int main( int argc, char* args[] )
{
    //Init SDL
    if(SDL_Init(SDL_INIT_EVERYTHING) < 0)
    {
        return 10;
    }
    //Creates a SDL Window
    if((window = SDL_CreateWindow("Image Loading", 100, 100, 320/*WIDTH*/, 320/*HEIGHT*/, SDL_WINDOW_RESIZABLE | SDL_RENDERER_PRESENTVSYNC)) == NULL)
    {
        return 20;
    }
    //SDL Renderer
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED );
    if (renderer == NULL)
    {
        std::cout << SDL_GetError() << std::endl;
        return 30;
    }

    //Init textures
    int w=0,h=0;
    background = IMG_LoadTexture(renderer,"splash.png");
    SDL_QueryTexture(background, NULL, NULL, &w, &h);
    imagen0 = IMG_LoadTexture(renderer,"imagen0.png");
    SDL_QueryTexture(imagen0, NULL, NULL, &w, &h);
    font = TTF_OpenFont( "Lazy.ttf", 28 );

    imagen0 = IMG_LoadTexture(renderer,"imagen0.png");
    rect_imagen0.x = 0; rect_imagen0.y = 0; rect_imagen0.w = w; rect_imagen0.h = h;
    rect_background.x = 0; rect_background.y = 0; rect_background.w = w; rect_background.h = h;

    char orientation = 'd';// d u l r
    int current_sprite = 0;
    int animation_velocity = 20;
    int velocity = 10;
    int frame = 0;
    map<char,vector<SDL_Texture*> >sprites;
    sprites['u'].push_back(IMG_LoadTexture(renderer, "personaje/up1.png"));
    sprites['u'].push_back(IMG_LoadTexture(renderer, "personaje/up2.png"));
    sprites['d'].push_back(IMG_LoadTexture(renderer, "personaje/down1.png"));
    sprites['d'].push_back(IMG_LoadTexture(renderer, "personaje/down2.png"));
    sprites['l'].push_back(IMG_LoadTexture(renderer, "personaje/left1.png"));
    sprites['l'].push_back(IMG_LoadTexture(renderer, "personaje/left2.png"));
    sprites['r'].push_back(IMG_LoadTexture(renderer, "personaje/right1.png"));
    sprites['r'].push_back(IMG_LoadTexture(renderer, "personaje/right2.png"));

    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2,2048);

    Mix_Music *music= Mix_LoadMUS("pokemon.wav");
    Mix_Chunk *sound= Mix_LoadWAV("colision.wav");

    SDL_QueryTexture(sprites['u'][0], NULL, NULL, &w, &h);
    rect_character.x = 128;
    rect_character.y = 64;
    rect_character.w = w;
    rect_character.h = h;

    texture_npc = IMG_LoadTexture(renderer,"npc.png");
    SDL_QueryTexture(texture_npc, NULL, NULL, &w, &h);
    rect_npc.x = 330;
    rect_npc.y = 79;
    rect_npc.w = w;
    rect_npc.h = h;

    texture_tile = IMG_LoadTexture(renderer,"tile/crypt.png");
    rect_tile.x = 32*4;
    rect_tile.y = 32*5;
    rect_tile.w = 32;
    rect_tile.h = 32;


    SDL_QueryTexture(texture_npc, NULL, NULL, &w, &h);
    rect_tileset.x = 0;
    rect_tileset.y = 0;
    rect_tileset.w = w;
    rect_tileset.h = h;

    map<string,vector<int> >mapas_down;
    map<string,vector<int> >mapas_over;
    map<string,vector<int> >mapas_collision;
    map<string,vector<Warp*> >warps;
            if(!Mix_PlayingMusic())
            Mix_PlayMusic(music,1);
    mapas_down["mapa0"]=getMapa("tile/test0.tmx",1);
    mapas_over["mapa0"]=getMapa("tile/test0.tmx",2);
    mapas_collision["mapa0"]=getMapa("tile/test0.tmx",3);
    warps["mapa0"] = getWarps("tile/test0.tmx");

    mapas_down["mapa1"]=getMapa("tile/test1.tmx",1);
    mapas_over["mapa1"]=getMapa("tile/test1.tmx",2);
    mapas_collision["mapa1"]=getMapa("tile/test1.tmx",3);
    warps["mapa1"] = getWarps("tile/test1.tmx");

    mapas_down["mapa2"]=getMapa("tile/test2.tmx",1);
    mapas_over["mapa2"]=getMapa("tile/test2.tmx",2);
    mapas_collision["mapa2"]=getMapa("tile/test2.tmx",3);
    warps["mapa2"] = getWarps("tile/test2.tmx");

    mapas_down["mapa3"]=getMapa("tile/test3.tmx",1);
    mapas_over["mapa3"]=getMapa("tile/test3.tmx",2);
    mapas_collision["mapa3"]=getMapa("tile/test3.tmx",3);
    warps["mapa3"] = getWarps("tile/test3.tmx");

    mapas_down["mapa4"]=getMapa("tile/test4.tmx",1);
    mapas_over["mapa4"]=getMapa("tile/test4.tmx",2);
    mapas_collision["mapa4"]=getMapa("tile/test4.tmx",3);
    warps["mapa4"] = getWarps("tile/test4.tmx");

    mapas_down["mapa5"]=getMapa("tile/test5.tmx",1);
    mapas_over["mapa5"]=getMapa("tile/test5.tmx",2);
    mapas_collision["mapa5"]=getMapa("tile/test5.tmx",3);
    warps["mapa5"] = getWarps("tile/test5.tmx");

    mapas_down["mapa6"]=getMapa("tile/test6.tmx",1);
    mapas_over["mapa6"]=getMapa("tile/test6.tmx",2);
    mapas_collision["mapa6"]=getMapa("tile/test6.tmx",3);
    warps["mapa6"] = getWarps("tile/test6.tmx");

    mapas_down["mapa7"]=getMapa("tile/test7.tmx",1);
    mapas_over["mapa7"]=getMapa("tile/test7.tmx",2);
    mapas_collision["mapa7"]=getMapa("tile/test7.tmx",3);
    warps["mapa7"] = getWarps("tile/test7.tmx");

    mapas_down["mapa8"]=getMapa("tile/test8.tmx",1);
    mapas_over["mapa8"]=getMapa("tile/test8.tmx",2);
    mapas_collision["mapa8"]=getMapa("tile/test8.tmx",3);
    warps["mapa8"] = getWarps("tile/test8.tmx");

    string mapa_actual="mapa0";

    //Main Loop
    while(true)
    {
        while(SDL_PollEvent(&Event))
        {
            if(Event.type == SDL_QUIT)
            {
                return 0;
            }
        }

        const Uint8* currentKeyStates = SDL_GetKeyboardState( NULL );

        if(currentKeyStates[ SDL_SCANCODE_RIGHT ])
        {
            if(collision(rect_character,rect_npc))
            Mix_PlayChannel(-1,sound,0);


            rect_character.x+=velocity;
            while(collision(rect_character,rect_npc))
            {
                rect_character.x-=1;
            }
            while(collisionLayer(mapas_collision[mapa_actual],rect_character))
            {
                rect_character.x-=1;
            }
            orientation='r';
        }
        if(currentKeyStates[ SDL_SCANCODE_SPACE ]&&(mapa_actual=="mapa0")){
                rect_character.x=warps["mapa1"][0]->x;
                rect_character.y=warps["mapa1"][0]->y;
                mapa_actual=warps["mapa1"][0]->mapa;
                rect_character.y=200;
                rect_npc.x=32;
                mapaa++;
        }
        if(currentKeyStates[ SDL_SCANCODE_LEFT ])
        {


            rect_character.x-=velocity;
            while(collision(rect_character,rect_npc))
            {
                rect_character.x+=1;
                showTTF("PresionaSPC",100);
            }
            while(collisionLayer(mapas_collision[mapa_actual],rect_character))
            {
                rect_character.x+=1;
                showTTF("PresionaSPC",100);
            }
            orientation='l';
        }
        if(currentKeyStates[ SDL_SCANCODE_DOWN ])
        {
            if(collision(rect_character,rect_npc))
            Mix_PlayChannel(-1,sound,0);
            rect_character.y+=velocity;
            while(collision(rect_character,rect_npc))
            {
                rect_character.y-=1;
            }

            while(collisionLayer(mapas_collision[mapa_actual],rect_character))
            {
                rect_character.y-=1;
            }
            orientation='d';
        }
        if(currentKeyStates[ SDL_SCANCODE_UP ])
        {
            if(collision(rect_character,rect_npc))
            Mix_PlayChannel(-1,sound,0);
            rect_character.y-=velocity;
            while(collision(rect_character,rect_npc))
            {
                rect_character.y+=1;
            }
            while(collisionLayer(mapas_collision[mapa_actual],rect_character))
            {
                rect_character.y+=1;
            }
            orientation='u';
        }
        if(currentKeyStates[ SDL_SCANCODE_RSHIFT ])
        {
            Mix_PlayChannel(-1,sound,0);
            velocity=6;
            animation_velocity=10;
        }else
        {
            velocity=3;
            animation_velocity=20;
        }

        if(frame%animation_velocity==0)
        {
            current_sprite++;
            if(current_sprite>1)
                current_sprite=0;
        }

//        currentKeyStates=NULL;
        for(int i=0;i<warps[mapa_actual].size();i++)
        {
            if(collision(warps[mapa_actual][i]->rect,rect_character))
            {
                rect_character.x=warps[mapa_actual][i]->x;
                rect_character.y=warps[mapa_actual][i]->y;
                mapa_actual=warps[mapa_actual][i]->mapa;


                if(mapaa==1){
                mapa_actual="mapa2";
                rect_character.y=200;
                rect_npc.x=132;
                rect_npc.y=132;
                //mapaa++;
                }

                if(mapaa==2){
                mapa_actual="mapa3";
                //rect_character.y+=200;
                rect_npc.x=32;
                rect_npc.y+=100;
                rect_character.y=200;

                }
                if(mapaa==3){
                mapa_actual="mapa4";
                rect_npc.y=+200;
                rect_character.y+=200;
                }
                if(mapaa==4){
                mapa_actual="mapa5";
                rect_character.y+=200;
                rect_npc.y+=100;
                }

                if(mapaa==5){
                mapa_actual="mapa6";
                rect_character.x=70;
                rect_character.y=122;
                rect_npc.x=32;
                rect_npc.y=90;
                }
                if(mapaa==6){
                mapa_actual="mapa7";
                rect_character.y=140;
                rect_npc.y+=65;

                }
                if(mapaa==7){

                mapa_actual="mapa8";
                rect_character.y+=300;

                }
                mapaa++;
            }
        }

        SDL_Delay(17);

        //if(mapa_actual=="mapa8")
        //background = IMG_LoadTexture(renderer,"ganar.png");
        //SDL_RenderCopy(renderer, background, NULL, &rect_background);


        dibujarLayer(renderer,mapas_down[mapa_actual]);


        SDL_RenderCopy(renderer, sprites[orientation][current_sprite], NULL, &rect_character);

        dibujarLayer(renderer,mapas_over[mapa_actual]);

//        dibujarLayer(renderer,collision_map);

        SDL_RenderCopy(renderer, texture_npc, NULL, &rect_npc);
        SDL_RenderPresent(renderer);
        SDL_RenderCopy(renderer, imagen0, NULL, &rect_imagen0);
        frame++;
        }

	return 0;
    }

