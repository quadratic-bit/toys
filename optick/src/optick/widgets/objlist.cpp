#include "./objlist.hpp"
#include "./desktop.hpp"

void ObjectsList::toggleSelect(ObjectPreview *preview) {
    bool toggled = preview->obj->toggleSelect();

    if (toggled) {
        if (selected) {
            selected->obj->unselect();
            selected->requestRedraw();
        }
        ObjectView *v = root->findChild<ObjectView>();
        if (v) v->populate(preview);
        selected = preview;
    } else {
        assert(selected == preview);
        assert(selected != NULL);
        selected->obj->unselect();
        ObjectView *v = root->findChild<ObjectView>();
        if (v) v->unpopulate();
        selected = NULL;
    }
}
