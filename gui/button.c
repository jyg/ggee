#include <m_pd.h>
#include "g_canvas.h"


#ifdef NT
#pragma warning( disable : 4244 )
#pragma warning( disable : 4305 )
#endif


typedef struct _button
{
     t_object x_obj;
     t_atom a_pos;

     t_glist * x_glist;
     int x_rect_width;
     int x_rect_height;
     t_symbol*  x_sym;
     t_symbol*  x_text;

     int x_height;
     int x_width;
} t_button;

/* widget helper functions */

#define DEBUG(x)


static void draw_inlets(t_button *x, t_glist *glist, int firsttime, int nin, int nout)
{
     int n = nin;
     int nplus, i;
     nplus = (n == 1 ? 1 : n-1);
     DEBUG(post("draw inlet");)
     for (i = 0; i < n; i++)
     {
	  int onset = text_xpix(&x->x_obj, glist) + (x->x_rect_width - IOWIDTH) * i / nplus;
	  if (firsttime)
	       sys_vgui(".x%x.c create rectangle %d %d %d %d -tags %xo%d\n",
			glist_getcanvas(glist),
			onset, text_ypix(&x->x_obj, glist) + x->x_rect_height - 2,
			onset + IOWIDTH, text_ypix(&x->x_obj, glist) + x->x_rect_height-1,
			x, i);
	  else
	       sys_vgui(".x%x.c coords %xo%d %d %d %d %d\n",
			glist_getcanvas(glist), x, i,
			onset, text_ypix(&x->x_obj, glist) + x->x_rect_height - 2,
			onset + IOWIDTH, text_ypix(&x->x_obj, glist) + x->x_rect_height-1);
     }
     n = nout; 
     nplus = (n == 1 ? 1 : n-1);
     for (i = 0; i < n; i++)
     {
	  int onset = text_xpix(&x->x_obj, glist) + (x->x_rect_width - IOWIDTH) * i / nplus;
	  if (firsttime)
	       sys_vgui(".x%x.c create rectangle %d %d %d %d -tags %xi%d\n",
			glist_getcanvas(glist),
			onset, text_ypix(&x->x_obj, glist)-2,
			     onset + IOWIDTH, text_ypix(&x->x_obj, glist)-1,
			x, i);
	  else
	       sys_vgui(".x%x.c coords %xi%d %d %d %d %d\n",
			glist_getcanvas(glist), x, i,
			onset, text_ypix(&x->x_obj, glist)-2,
			onset + IOWIDTH, text_ypix(&x->x_obj, glist)-1);
	  
     }
     DEBUG(post("draw inlet end");)
}


static void draw_handle(t_button *x, t_glist *glist, int firsttime) {
  int onset = text_xpix(&x->x_obj, glist) + (x->x_rect_width - IOWIDTH+2);

  if (firsttime)
    sys_vgui(".x%x.c create rectangle %d %d %d %d -tags %xhandle\n",
	     glist_getcanvas(glist),
	     onset, text_ypix(&x->x_obj, glist) + x->x_rect_height - 12,
	     onset + IOWIDTH-2, text_ypix(&x->x_obj, glist) + x->x_rect_height-4,
	     x);
  else
    sys_vgui(".x%x.c coords %xhandle %d %d %d %d\n",
	     glist_getcanvas(glist), x, 
	     onset, text_ypix(&x->x_obj, glist) + x->x_rect_height - 12,
	     onset + IOWIDTH-2, text_ypix(&x->x_obj, glist) + x->x_rect_height-4);
}

static void create_widget(t_button *x, t_glist *glist)
{
  char text[MAXPDSTRING];
  int len,i;
  t_canvas *canvas=glist_getcanvas(glist);
  x->x_rect_width = x->x_width+10;
  x->x_rect_height =  x->x_height*20+12;

  strncpy(text,x->x_text->s_name,MAXPDSTRING);
  len = strlen(text);
  for (i=0;i<len;i++) {
    if (text[i] == '_')
      text[i] = ' ';
  }
  
  sys_vgui("button .x%x.c.s%x \
                    -height %d \
                    -text \"%s\" \
                    -command button_cb%x\n",canvas,x,
	   x->x_height,text,
	   x);
}





static void button_drawme(t_button *x, t_glist *glist, int firsttime)
{
  t_canvas *canvas=glist_getcanvas(glist);
  DEBUG(post("drawme %d",firsttime);)
     if (firsttime) {
       DEBUG(post("glist %x canvas %x",x->x_glist,canvas);)
       if (x->x_glist != canvas) {
	 create_widget(x,glist);	       
	 x->x_glist = canvas;
       }
       sys_vgui(".x%x.c create window %d %d -anchor nw -window .x%x.c.s%x -tags %xS\n", 
		canvas,text_xpix(&x->x_obj, glist), text_ypix(&x->x_obj, glist),x->x_glist,x,x);
              
     }     
     else {
       sys_vgui(".x%x.c coords %xS \
%d %d\n",
		canvas, x,
		text_xpix(&x->x_obj, glist), text_ypix(&x->x_obj, glist));
     }
     draw_inlets(x, glist, firsttime, 1,1);
     draw_handle(x, glist, firsttime);

}


static void button_erase(t_button* x,t_glist* glist)
{
     int n;

     DEBUG(post("erase");)
     sys_vgui("destroy .x%x.c.s%x\n",glist_getcanvas(glist),x);

     sys_vgui(".x%x.c delete %xS\n",glist_getcanvas(glist), x);

     /* inlets and outlets */
     
     sys_vgui(".x%x.c delete %xi%d\n",glist_getcanvas(glist),x,0);
     sys_vgui(".x%x.c delete %xo%d\n",glist_getcanvas(glist),x,0);
     sys_vgui(".x%x.c delete  %xhandle\n",glist_getcanvas(glist),x,0);
}
	


/* ------------------------ button widgetbehaviour----------------------------- */


static void button_getrect(t_gobj *z, t_glist *owner,
    int *xp1, int *yp1, int *xp2, int *yp2)
{
    int width, height;
    t_button* s = (t_button*)z;

    width = s->x_rect_width;
    height = s->x_rect_height;
    *xp1 = text_xpix(&s->x_obj, owner);
    *yp1 = text_ypix(&s->x_obj, owner) - 1;
    *xp2 = text_xpix(&s->x_obj, owner) + width;
    *yp2 = text_ypix(&s->x_obj, owner) + height;
}

static void button_displace(t_gobj *z, t_glist *glist,
    int dx, int dy)
{
    t_button *x = (t_button *)z;
    DEBUG(post("displace");)
    x->x_obj.te_xpix += dx;
    x->x_obj.te_ypix += dy;
    if (glist_isvisible(glist))
    {
      sys_vgui(".x%x.c coords %xSEL %d %d %d %d\n",
	       glist_getcanvas(glist), x,
	       text_xpix(&x->x_obj, glist), text_ypix(&x->x_obj, glist)-1,
	       text_xpix(&x->x_obj, glist) + x->x_rect_width, text_ypix(&x->x_obj, glist) + x->x_rect_height-2);
      
      button_drawme(x, glist, 0);
      canvas_fixlinesfor(glist_getcanvas(glist),(t_text*) x);
    }
    DEBUG(post("displace end");)
}

static void button_select(t_gobj *z, t_glist *glist, int state)
{
     t_button *x = (t_button *)z;
     if (state) {
	  sys_vgui(".x%x.c create rectangle \
%d %d %d %d -tags %xSEL -outline blue\n",
		   glist_getcanvas(glist),
		   text_xpix(&x->x_obj, glist), text_ypix(&x->x_obj, glist)-1,
		   text_xpix(&x->x_obj, glist) + x->x_rect_width, text_ypix(&x->x_obj, glist) + x->x_rect_height-2,
		   x);
     }
     else {
	  sys_vgui(".x%x.c delete %xSEL\n",
		   glist_getcanvas(glist), x);
     }



}


static void button_activate(t_gobj *z, t_glist *glist, int state)
{
/*    t_text *x = (t_text *)z;
    t_rtext *y = glist_findrtext(glist, x);
    if (z->g_pd != gatom_class) rtext_activate(y, state);*/
}

static void button_delete(t_gobj *z, t_glist *glist)
{
    t_text *x = (t_text *)z;
    canvas_deletelinesfor(glist_getcanvas(glist), x);
}

       
static void button_vis(t_gobj *z, t_glist *glist, int vis)
{
    t_button* s = (t_button*)z;
    t_rtext *y;
    DEBUG(post("vis: %d",vis);)
    if (vis) {
      	y = (t_rtext *) rtext_new_without_senditup(glist, (t_text *)z, glist->gl_editor->e_rtext);
	 button_drawme(s, glist, 1);
    }
    else {
	y = glist_findrtext(glist, (t_text *)z);
	 button_erase(s,glist);
	rtext_free(y);
    }
}

static void button_save(t_gobj *z, t_binbuf *b);

t_widgetbehavior   button_widgetbehavior = {
  w_getrectfn:  button_getrect,
  w_displacefn: button_displace,
  w_selectfn:   button_select,
  w_activatefn: button_activate,
  w_deletefn:   button_delete,
  w_visfn:      button_vis,
  w_savefn:     button_save,
  w_clickfn:    NULL,
  w_propertiesfn: NULL,
}; 


void button_size(t_button* x,t_floatarg w,t_floatarg h) {
     x->x_width = w;
     x->x_height = h;
}

void button_color(t_button* x,t_symbol* col)
{
/*     outlet_bang(x->x_obj.ob_outlet); only bang if there was a bang .. 
       so color black does the same as bang, but doesn't forward the bang 
*/
}

static void button_bang(t_button* x)
{
  sys_vgui(".x%x.c.s%x flash\n",x->x_glist,x);
  outlet_bang(x->x_obj.ob_outlet); 
}


static void button_b(t_button* x)
{
  outlet_bang(x->x_obj.ob_outlet); 
}



static void button_save(t_gobj *z, t_binbuf *b)
{

    t_button *x = (t_button *)z;

    binbuf_addv(b, "ssiiss", gensym("#X"),gensym("obj"),
		x->x_obj.te_xpix, x->x_obj.te_ypix ,  
		gensym("button"),x->x_text);
    binbuf_addv(b, ";");
}


static t_class *button_class;



static void *button_new(t_symbol* text)
{
    t_button *x = (t_button *)pd_new(button_class);
    char buf[256];

    x->x_glist = (t_glist*)NULL;

    x->x_width = 20;
    x->x_height = 1;
    if (text == &s_)
      x->x_text = gensym("OK");
    else
      x->x_text = text;

    /* TODO .. ask the button for its width */
    x->x_width += strlen(x->x_text->s_name)*5.7;

    sprintf(buf,"button%x",x);
    x->x_sym = gensym(buf);
    pd_bind(&x->x_obj.ob_pd, x->x_sym);

/* pipe startup code to tk */

    sys_vgui("proc button_cb%x {} {\n
       pd [concat button%x b \\;]\n
       }\n",x,x);

    outlet_new(&x->x_obj, &s_float);
    return (x);
}

void button_setup() {
  post("button setup");
    button_class = class_new(gensym("button"), (t_newmethod)button_new, 0,
				sizeof(t_button),0,A_DEFSYM,0);

    class_addbang(button_class, (t_method)button_bang);
    class_addmethod(button_class, (t_method)button_b,gensym("b"),0);

    class_setwidget(button_class,&button_widgetbehavior);

}


