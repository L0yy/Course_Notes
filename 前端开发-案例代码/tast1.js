isc.Page.setAppImgDir("sc/skins/EnterpriseBlue/");

var adminListData = [{
		"username": "admin",
		"authType": "local",
		"loginType": ["telnet", "ssh", "web"],
		"userType": "Administrator",
		"comment": "admin user"
	}, {
		"username": "auditor",
		"allowVsys": ["root"],
		"authType": "external",
		"loginType": ["ssh"],
		"userType": "Auditor",
		"comment": "auditor user"
	}
];

isc.DynamicForm.create({
	ID: "basicForm",
	numCols: 3,
	//width:200,
	//colWidths: [30,30,180],
	cellBorder:1,
	items: [{
			type: "TextItem",
			name: "name",
			title: "名称",
			titleAlign: "left",
			titleColSpan: 2,
			width: 180,
			wrapTitle: false,
			required: true
		}, {
			type: "TextAreaItem",
			name: "comment",
			title: "描述",
			titleAlign: "left",
			titleColSpan: 2,
			wrapTitle: false
		}, {
			type: "RadioGroupItem",
			name: "loginType",
			title: "认证类型",
			titleAlign: "left",
			titleColSpan: 2,
			vertical: false,
			valueMap: {
				"local": "本地",
				"external": "外部"
			},
			defaultValue: "local",
		}, {
			type: "PasswordItem",
			name: "password",
			title: "密码",
			titleAlign: "left",
			titleColSpan: 2,
			validators: [{
					type: "lengthRange",
					min: 6,
					max: 128,
					errorMessage: "密码长度为6-128",
					stopIfFalse: true
				}, {
					type: "matchesField",
					otherField: "confirmPassword",
					errorMessage: "密码与确认密码一致",
					stopIfFalse: true
				}
			]
		}, {
			type: "PasswordItem",
			name: "confirmPassword",
			title: "确认密码",
			titleAlign: "left",
			titleColSpan: 2,
			validators: [{
					type: "lengthRange",
					min: 6,
					max: 128,
					errorMessage: "密码长度为6-128",
					stopIfFalse: true
				}, {
					type: "matchesField",
					otherField: "password",
					errorMessage: "密码与确认密码一致",
					stopIfFalse: true
				}
			]
		}, {
			type: "CheckboxItem",
			name: "telnet",
			title: "Telnet",
			showTitle: false,
			width: 50,
			valueIconLeftPadding: 0,
			defaultValue: false
		}, {
			type: "CheckboxItem",
			name: "ssh",
			title: "SSH",
			showTitle: false,
			width: 50,
			valueIconLeftPadding: 0,
			defaultValue: false
		}, {
			type: "CheckboxItem",
			name: "web",
			title: "Web",
			showTitle: false,
			valueIconLeftPadding: 0,
			defaultValue: false
		}, {
			type: "NESelectItem",
			name: "userType",
			title: "用户类型",
			titleAlign: "left",			
			titleColSpan: 2,
			valueMap: {
				"Administrator": "Administrator",
				"Auditor": "Auditor"
			},
			wrap: false
		}
	]
});

isc.VLayout.create({
	ID: "vlayout",
	layoutLeftMargin: 30,
	layoutTopMargin: 20,
	members: [basicForm, isc.LayoutSpacer.create({})]

});
