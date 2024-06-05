#include <stdio.h>
#include <stdlib.h>

#define CONFIG_XML_PATH    "./test.xml"

extern void create_doc(char *path);
extern void parse_doc(char *path);
extern int search_xml_node(char *path);


int main()
{
    create_doc(CONFIG_XML_PATH);
    parse_doc(CONFIG_XML_PATH);

    search_xml_node(CONFIG_XML_PATH);

    system("pause");
    return 0;
}
