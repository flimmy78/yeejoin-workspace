/*
 * �����嵥��texbox�ؼ���ʾ
 *
 * ������ӻ��ڴ�������view�����Ӽ�����ͬ���͵�textbox�ؼ�
 */
#include "demo_view.h"
#include <rtgui/widgets/label.h>
#include <rtgui/widgets/textbox.h>

/* ����������ʾtextbox�ؼ�����ͼ */
rtgui_view_t* demo_view_textbox(rtgui_workbench_t* workbench)
{
	rtgui_rect_t rect, textbox_rect;
	rtgui_view_t* view;
	rtgui_label_t* label;
	rtgui_textbox_t* text;

	/* �ȴ���һ����ʾ�õ���ͼ */
	view = demo_view(workbench, "TextBox View");

	/* �����ͼ��λ����Ϣ */
	demo_view_get_rect(view, &rect);
	rect.x1 += 5;
	rect.x2 = rect.x1 + 30;
	rect.y1 += 5;
	rect.y2 = rect.y1 + 20;
	/* ����һ��label�ؼ� */
	label = rtgui_label_create("����: ");
	/* ����label��λ�� */
	rtgui_widget_set_rect(RTGUI_WIDGET(label), &rect);
	/* view��һ��container�ؼ�������add_child�����������label�ؼ� */
	rtgui_container_add_child(RTGUI_CONTAINER(view), RTGUI_WIDGET(label));

	/* ��textbox_rect��ֵ��rect���Լ���textbox�ؼ���λ�� */
	textbox_rect = rect;
	textbox_rect.x1 = textbox_rect.x2 + 5;
	textbox_rect.x2 = textbox_rect.x1 + 160;
	/* ����һ��textbox�ؼ� */
	text = rtgui_textbox_create("David",RTGUI_TEXTBOX_SINGLE);
	/* ����textbox�ؼ���λ�� */
	rtgui_widget_set_rect(RTGUI_WIDGET(text), &textbox_rect);
	/* ����textbox�ؼ�����ͼ�� */
	rtgui_container_add_child(RTGUI_CONTAINER(view), RTGUI_WIDGET(text));

	/* ������һ��label�ؼ���λ�� */
	rect.y1 += 23;
	rect.y2 = rect.y1 + 20;
	/* ����һ��label�ؼ� */
	label = rtgui_label_create("�ʼ�: ");
	/* ����label��λ�� */
	rtgui_widget_set_rect(RTGUI_WIDGET(label), &rect);
	/* ����label�ؼ�����ͼ�� */
	rtgui_container_add_child(RTGUI_CONTAINER(view), RTGUI_WIDGET(label));
	textbox_rect = rect;
	textbox_rect.x1 = textbox_rect.x2 + 5;
	textbox_rect.x2 = textbox_rect.x1 + 160;
	/* ����һ��textbox�ؼ� */
	text = rtgui_textbox_create("zhaoshaowei@yeejoin.com",RTGUI_TEXTBOX_SINGLE);
	/* ����textbox�ؼ���λ�� */
	rtgui_widget_set_rect(RTGUI_WIDGET(text), &textbox_rect);
	/* ����textbox�ؼ�����ͼ�� */
	rtgui_container_add_child(RTGUI_CONTAINER(view), RTGUI_WIDGET(text));

	rect.y1 += 23;
	rect.y2 = rect.y1 + 20;
	/* ����һ��label�ؼ� */
	label = rtgui_label_create("����: ");
	/* ����label��λ�� */
	rtgui_widget_set_rect(RTGUI_WIDGET(label), &rect);
	/* ����label�ؼ�����ͼ�� */
	rtgui_container_add_child(RTGUI_CONTAINER(view), RTGUI_WIDGET(label));
	textbox_rect = rect;
	textbox_rect.x1 = textbox_rect.x2 + 5;
	textbox_rect.x2 = textbox_rect.x1 + 160;
	/* ����һ��textbox�ؼ� */
	text = rtgui_textbox_create("yeejoin",RTGUI_TEXTBOX_SINGLE);
	/* ����textbox��ʾ�ı�Ϊ������ʽ(����ʾΪ*�ţ��ʺ�����ʾ��������) */
	text->flag |= RTGUI_TEXTBOX_MASK;
	/* ����textbox�ؼ���λ�� */
	rtgui_widget_set_rect(RTGUI_WIDGET(text), &textbox_rect);
	/* ����textbox�ؼ�����ͼ�� */
	rtgui_container_add_child(RTGUI_CONTAINER(view), RTGUI_WIDGET(text));

	rect.y1 += 23;
	rect.y2 = rect.y1 + 20;
	/* ����һ��label�ؼ� */
	label = rtgui_label_create("��ҳ: ");
	/* ����label��λ�� */
	rtgui_widget_set_rect(RTGUI_WIDGET(label), &rect);
	/* ����label�ؼ�����ͼ�� */
	rtgui_container_add_child(RTGUI_CONTAINER(view), RTGUI_WIDGET(label));
	textbox_rect = rect;
	textbox_rect.x1 = textbox_rect.x2 + 5;
	textbox_rect.x2 = textbox_rect.x1 + 160;
	/* ����һ��textbox�ؼ� */
	text = rtgui_textbox_create("www.yeejoin.com",RTGUI_TEXTBOX_SINGLE);
	/* ����textbox�ؼ���λ�� */
	rtgui_widget_set_rect(RTGUI_WIDGET(text), &textbox_rect);
	/* ����textbox�ؼ�����ͼ�� */
	rtgui_container_add_child(RTGUI_CONTAINER(view), RTGUI_WIDGET(text));

	return view;
}