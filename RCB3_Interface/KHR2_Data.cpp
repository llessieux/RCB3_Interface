#include "stdafx.h"
#include "KHR2_Data.h"

RCBMotion::RCBMotion()
{
    m_type = 0;
    m_start = 0;
    m_current_pos = 0;
    m_control = 0;
    m_item_count = 0;
    m_link_count = 0;
    m_name = NULL;
}

bool RCBMotion::SaveToFile(RCBMotion *m,char *filename)
{
    FILE *f = fopen(filename,"wt");

    if ( f == NULL )
        return false;

    fprintf(f,"[GraphicalEdit]\n");
    fprintf(f,"Type=%d\n",m->m_type);
    fprintf(f,"Width=500\n");
    fprintf(f,"Height=%d\n",30*((m->m_item_count/8) + 1));
    fprintf(f,"Items=%d\n",m->m_item_count);
    fprintf(f,"Links=%d\n",m->m_link_count);
    fprintf(f,"Start=%d\n",m->m_start);
    fprintf(f,"Name=%s\n",m->m_name);
    fprintf(f,"Port=0\n");
    fprintf(f,"Ctrl=%d\n",m->m_control);
    fprintf(f,"\n");

    for(int i=0;i<m->m_item_count;i++)
    {
        const RCBMotionItem &item = m->m_items[i];
        fprintf(f,"[Item%d]\n",i);
        fprintf(f,"Name=%s\n",item.m_name);
        fprintf(f,"Width=%d\n",item.m_width);
        fprintf(f,"Height=%d\n",item.m_height);
        fprintf(f,"Left=%d\n",item.m_left);
        fprintf(f,"Top=%d\n",item.m_top);
        fprintf(f,"Color=%d\n",item.m_color);
        fprintf(f,"Type=%d\n",item.m_type);
        fprintf(f,"Prm=");
        std::list<int>::const_iterator it = item.m_params.begin();
        for(int j=0;j<24;j++,it++)
        {
            fprintf(f,"%d",*it);
            if ( j != 23 )
                fprintf(f,",");
            else
                fprintf(f,"\n");
        }
        fprintf(f,"\n");
    }


    for(int i=0;i<m->m_link_count;i++)
    {
        const RCBMotionLink &item = m->m_links[i];
        fprintf(f,"[Link%d]\n",i);
        fprintf(f,"Main=%d\n",item.m_main);
        fprintf(f,"Origin=%d\n",item.m_origin);
        fprintf(f,"Final=%d\n",item.m_final);
        fprintf(f,"Point=");
        for(unsigned int j=0;j<item.m_points.size();j++)
        {
            fprintf(f,"%d",item.m_points[j]);
            if ( j != item.m_points.size()-1 )
                fprintf(f,",");                
        }
        fprintf(f,"\n");
        fprintf(f,"\n");
    }

    fclose(f);
    return true;
}


bool RCBMotion::ReadFromFile(const char *filename)
{
    if ( !filename )
        return false;

    FILE *f = fopen(filename,"r");

    if ( f == NULL )
        return false;

    m_name = _strdup(filename);
    int status = 0;
    int link_index = 0;
    int item_index = 0;
    char buffer[512];

    RCBMotionLink link;
    RCBMotionItem item;
    
    item.m_name = NULL;
    while(!feof(f))
    {
        memset(buffer,0,512);
        fgets(buffer,512,f);
        if ( buffer[0] == '[' )
        {
            if ( status == 2) {
                m_items[item_index] = item;
                m_items[item_index].m_name = item.m_name;
                item.m_name = NULL;
                item.m_params.clear();
            }
            else if ( status == 3 ){
                m_links[link_index] = link;
                link.m_points.clear();
            }

            if ( strstr(buffer+1,"GraphicalEdit") != NULL )
            {
                status = 1;
            }
            else if ( strstr(buffer+1,"Item") != NULL )
            {
                status = 2;
                item_index = atoi(buffer+5);
                item.m_index = item_index;
            }
            else if ( strstr(buffer+1,"Link") != NULL )
            {
                status = 3;
                link_index = atoi(buffer+5);
                link.m_index = link_index;
            }
        }
        else {
            //Reading: GraphicalEdit
            if ( status == 1 )
            {
                if ( strstr(buffer,"Type") != NULL )
                {
                    m_type = atoi(buffer+5);
                }
                else if ( strstr(buffer,"Items") != NULL )
                {
                    m_item_count = atoi(buffer+6);
                    m_items.resize(m_item_count);
                }
                else if ( strstr(buffer,"Links") != NULL )
                {
                    m_link_count = atoi(buffer+6);
                    m_links.resize(m_link_count);
                }
                else if ( strstr(buffer,"Start") != NULL )
                {
                    m_start = atoi(buffer+6);
                }
                else if ( strstr(buffer,"Ctrl") != NULL )
                {
                    m_control = atoi(buffer+5);
                }
            }
            else  if ( status == 2 ) //Reading: Item
            {
                if ( strncmp(buffer,"Width",5) == 0 )
                {
                    item.m_width = atoi(buffer+6);
                }
                else if ( strncmp(buffer,"Height",6) == 0 )
                {
                    item.m_height = atoi(buffer+7);
                }
                else if ( strncmp(buffer,"Left",4) == 0 )
                {
                    item.m_left = atoi(buffer+5);
                }
                else if ( strncmp(buffer,"Top",3) == 0 )
                {
                    item.m_top = atoi(buffer+4);
                }
                else if ( strncmp(buffer,"Color",5) == 0 )
                {
                    item.m_color = atoi(buffer+6);
                }
                else if ( strncmp(buffer,"Type",4) == 0 )
                {
                    item.m_type = atoi(buffer+5);
                }
                else if ( strncmp(buffer,"Name",4) == 0 )
                {
                    item.m_name = _strdup(buffer+5);
                }
                else if ( strncmp(buffer,"Prm",3) == 0 )
                {
                    char *token = strtok(buffer+4,",");
                    while( token )
                    {
                        int v = atoi(token);
                        token = strtok(NULL,",");
                        item.m_params.push_back(v);
                    }
                }
            }
            else  if ( status == 3 ) //Reading: Link
            {
                if ( strstr(buffer,"Origin") != NULL )
                {
                    link.m_origin = atoi(buffer+7);
                }
                else if ( strstr(buffer,"Point") != NULL )
                {
                    char *token = strtok(buffer+6,", ");                   
                    while( token )
                    {
                        if ( isdigit(*token) )
                        {
                            int v = atoi(token);
                            token = strtok(NULL,",");
                            link.m_points.push_back(v);
                        }
                        else
                            token = strtok(NULL,",");
                    }
                }
                else if ( strstr(buffer,"Final") != NULL )
                {
                    link.m_final = atoi(buffer+6);
                }
                else if ( strstr(buffer,"Main") != NULL )
                {
                    link.m_main = atoi(buffer+5);
                }

            }
        }
    }

    if ( status == 2) {
        m_items[item_index] = item;
        m_items[item_index].m_name = item.m_name;
        item.m_params.clear();
    }
    else if ( status == 3 ){
        m_links[link_index] = link;
    }

    m_current_pos = m_start;
    fclose(f);

    return true;

}
unsigned int
RCBMotion::GetNextItem(bool default_jump)
{
    bool found = false;
    for(int l=0;l<m_link_count;l++)
    {
        const RCBMotionLink &link = m_links[l];
        if ( link.m_origin == m_current_pos )
        {                        
            if ( default_jump && (link.m_main == 0))
            {
                m_current_pos = link.m_final;
                found = true;
                break;
            }
            if ( !default_jump && (link.m_main != 0))
            {
                m_current_pos = link.m_final;
                found = true;
                break;
            }
        }
    }
    if ( !found )
    {
        //Reset it to point at the start.
        m_current_pos = 0;
        return NO_MOTION_INDEX;
    }

    return m_current_pos;
}


