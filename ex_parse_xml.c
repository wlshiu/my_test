#include <stdio.h>
#include <stdlib.h>

#include "libxml/parser.h"

xmlChar *bridge_name = NULL;

/* 遞迴遍歷 */
void parse_node(xmlNodePtr node, int space)
{
    xmlNodePtr cur;
    xmlAttr *attr;

    /* 解析元素 */
    if (node->type == XML_ELEMENT_NODE)
    {
        int i;
        printf("\n");
        for (i = 0; i < space; i++)
        {
            printf(" ");
        }
        printf("%s", node->name);

        /* 例子：尋找attr */
        if (xmlStrcmp(node->name, BAD_CAST("bridge")) == 0)
        {
            bridge_name = xmlGetProp(node, BAD_CAST("name"));
        }

        cur = node->children;
        if (cur != NULL && cur->type == XML_TEXT_NODE && cur->next == NULL && cur->prev == NULL)
            printf(": \"%s\"", cur->content);

        i = 0;
        attr = node->properties;
        while (attr != NULL)
        {
            /*
             * attr->children是一個xmlNode，文字類型，這個函數取該xmlNode的content
             * 後面會有更清晰的例子
             * */
            xmlChar *str = xmlNodeListGetString(node->doc, attr->children, 1);
            if (i == 0)
            {
                i = 1;
                printf("(%s=\"%s\"", attr->name, str);
            }
            else
            {
                printf(" %s=\"%s\"", attr->name, str);
            }
            free(str);
            attr = attr->next;
        }
        if (i != 0)
        {
            printf(")");
        }

        cur = node->children;
        while (cur != NULL)
        {
            /* 遞迴 */
            parse_node(cur, space + 2);
            cur = cur->next;
        }
    }
    else if (node->type == XML_TEXT_NODE)
    {
        /* 例子，XML格式化造成的多餘content */
        if (node->next != NULL || node->prev != NULL)
        {
            int i;
            printf("\n");
            for (i = 0; i < space; i++)
            {
                printf(" ");
            }
            printf("# USELESS CONTENT #");
        }
    }
}


void parse_doc(char *path)
{
    xmlDocPtr doc;
    xmlNodePtr root;

    doc = xmlParseFile(path);

    if (doc == NULL )
    {
        fprintf(stderr, "failed to parse file \"%s\".\n", path);
        return;
    }

    root = xmlDocGetRootElement(doc);

    if (root == NULL)
    {
        fprintf(stderr, "empty document\n");
        xmlFreeDoc(doc);
        return;
    }

    parse_node(root, 0);
    printf("\n\n");

    /* 尋找attr的結果 */
    if (bridge_name)
    {
        printf("find bridge attr name: %s\n", bridge_name);
        free(bridge_name);
    }

    xmlFreeDoc(doc);
    return;
}

/* xmlNodeListGetString，可以看到返回了"c1"和"c2"兩個字串拼接的結果 */
void test()
{
    xmlDocPtr doc;
    xmlNodePtr node;
    xmlChar *str = BAD_CAST("<e1>c1<e2/>c2</e1>");

    doc = xmlParseDoc(str);
    node = xmlDocGetRootElement(doc);
    str = xmlNodeListGetString(doc, node->children, 1);
    printf("test example: \"%s\"\n", str);
    free(str);
    xmlFreeDoc(doc);
}


//int main(int argc, char **argv)
//{
//
//    if (argc < 2)
//    {
//        fprintf(stderr, "Usage: %s filename\n", argv[0]);
//        return -1;
//    }
//
//    parse_doc(argv[1]);
//
//    test();
//
//    return 0;
//}
