isc.ListGrid.create({ //form的嵌套的list
	ID: "listgrid",
	fields: [{
			name: "flag",
			title: "Flag"
		}, {
			name: "country",
			title: "country"
		}
	],
	data: [{
			"flag": "JA",
			"country": "Japan"
		}, {
			"flag": "CH",
			"country": "CHINA"
		}
	]
});

isc.DynamicForm.create({
	numCols: 4,
	colWidths: [80, 150, 80, 150],
	cellBorder: 1,
	padding: 10,
	border: "1px red solid",
	cellPadding: 10,
	width: 700,
	ID: "DynamicForm0",
	visiable: true,
	wrapItemTitles: false,
	fields: [{
			name: "TextItem00000",
			type: "TextItem",
			title: "TextItem",
			width: "100",
			value: "123",
			hint: "<nobr>(1-255)</nobr>",
			titleAlign: "left",
			height: 50,
			required: true,
			requiredMessage: "not null"
		}, {
			name: "PasswordItem0",
			type: "password",
			required: true
		}, {
			name: "TextAreaItem0",
			type: "TextAreaItem"
		}, {
			name: "SpacerItem0",
			type: "SpacerItem",
			colSpan: "*"
		}, {
			name: "StaticTextItem0",
			type: "StaticTextItem",
			defaultValue: "StaticItem",
			showTitle: false,
			colSpan: 2
		}, {
			name: "getSpacerItem",
			type: "ButtonItem",
			startRow: false,
			endRow: false,
			click: function (form, item, value) {
				alert(form.getItem("SpacerItem0"));
				form.getField("TextAreaItem0").setValue(form.getValue("TextItem0"));
			}
		}, {
			name: "showButton",
			type: "CheckboxItem",
			showTitle: false,
			changed: function (form, item, value) {
				form.getField("ClearValues").show();
				alert("button's visibility is " + form.getField("ClearValues").isVisible());
			}
		}, {
			valueMap: ["yes", "no"],
			defaultValue: "yes",
			name: "FocusInItem",
			type: "RadioGroupItem",
			changed: function (form, value) {
				var val = form.getValue("FocusInItem");
				if (val == "yes") {
					form.focusInItem("TextAreaItem0");
				} else {
					form.focusInItem("PasswordItem0");
				}
			}
		}, {
			valueMap: ["a", "b"],
			name: "SelectItem0",
			type: "SelectItem"
		}, {
			name: "RowSpacerItem0",
			type: "RowSpacerItem"
		}, {
			name: "DateItem0",
			type: "DateItem"
		}, {
			name: "TimeItem0",
			type: "TimeItem"
		}, {
			name: "DateItem1",
			type: "DateItem",
			useTextField: true,
			colSpan: "*"
		}, {
			name: "ClearValues",
			type: "ButtonItem",
			colSpan: "*",
			visible: false,
			click: function (form, item, value) {
				form.clearValues();
			}
		}, {
			name: "upload",
			type: "upload"
		}, {
			name: "a",
			type: "CanvasItem",
			canvas: "listgrid",
			titleVAlign: "top"
		}, {
			name: "okButton",
			type: "ButtonItem",
			title: "OK",
			startRow: false,
			endRow: false,
			click: function (form, value) {
				//form.disableValidation();添加了之后则下面的校验语句不再起作用
				form.validate();
				alert("hasErrors: " + form.hasErrors());
			}
		}, {
			name: "resetButton",
			type: "ButtonItem",
			title: "Reset",
			startRow: false,
			endRow: false,
			click: function (form, value) {
				form.reset();
			}
		}, {
			name: "clearButton",
			type: "ButtonItem",
			title: "ClearErrors",
			startRow: false,
			endRow: false,
			click: function (form, value) {
				form.clearErrors(true);
			}
		}, {
			name: "hideButton",
			type: "ButtonItem",
			title: "HideButton",
			startRow: false,
			endRow: false,
			click: function (form, value) {
				alert("ClearValues Button will be hidden");
				form.hideItem("ClearValues");
			}
		}
	]
});
