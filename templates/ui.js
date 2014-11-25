
/* * * * * * * * * * * * * * * * * * * * * * * * *
 *             TabControl class
 * * * * * * * * * * * * * * * * * * * * * * * * */




function tab_control_create(obj) {
  var tc = obj.doc.getElementById(obj.id);
  var html = "";
  var changed_str = obj.changed ? "_changed" : "";

  html =
    "<table>" +
    "<tr>";
  for (var i = 0; i < obj.elements.length; ++i) {
    html +=
      "<td class=\"tab_control_item" + changed_str + "_toggle_off\" " +
      "onclick=\"tab_control_change(" + obj.id + ", " + i + ")\">" +
      obj.elements[i].caption + "</td>";
    obj.on_select_change(obj, i);
  }
  html +=
    "</tr>" +
    "</table>";

  tc.innerHTML = html;
}

function tab_control_change(obj, selected_index) {
  obj.changed = true;
  tab_control_represent_state(obj, selected_index);
}

function tab_control_set_selected(obj, selected_index) {
  obj.changed = false;
  tab_control_represent_state(obj, selected_index);
}

function tab_control_represent_state(obj, selected_index) {
  var tc = obj.doc.getElementById(obj.id);
  var tbl = tc.childNodes[0];
  var cells = tbl.childNodes[0].childNodes[0].childNodes;
  var text = "";
  var changed_str = obj.changed ? "_changed" : "";

  obj.selected_index = selected_index;

  for (var i = 0; i < cells.length; ++i) {
    if (i == selected_index) {
      cells[i].className = "tab_control_item" +
          changed_str + "_toggle_on";
    }
    else {
      cells[i].className = "tab_control_item" +
          changed_str + "_toggle_off";
    }
    obj.on_select_change(obj, i);
  }
}

/* vim: set expandtab ts=2 : */
