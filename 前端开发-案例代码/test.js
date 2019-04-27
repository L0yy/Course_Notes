isc.Page.setAppImgDir("sc/skins/EnterpriseBlue/");

var listGridData = [{
			"flag": "JA",
			"country": "Japan"
		}, {
			"flag": "CH",
			"country": "CHINA"
		}];

isc.DynamicForm.create({
	ID:"testDynamicForm",
    autoFocus: true,
    numCols: 3,
    items:[
        {
            type: "TextItem",
            name: "you",
            title: "Enter your name",
            selectOnFocus: true,
            wrapTitle: false,
            defaultValue: "my friend"
        },
        {
            type: "ButtonItem",
            title: "Hello",
            width: 80,
            startRow: false,
            icon: "img/message.png",
            click: "isc.say('Hello ' + form.getValue('you') + '!')"
        }
    ]
});

isc.ListGrid.create({
	ID: "listgrid",
	width: 200,
	minHeight: 100,
	fields: [{
			name: "flag",
			title: "Flag"
		}, {
			name: "country",
			title: "country"
		}
	]
});

isc.Button.create({
	ID: "buttonID",
	title: "Set ListGrid Data",
    width: 150,
	click: function(){
		listgrid.setData(listGridData);
	}
});

isc.VLayout.create({
	ID:"vlayout",
	membersMargin: 10,
	layoutLeftMargin: 30,
	layoutTopMargin: 20,
	members:[testDynamicForm,listgrid,buttonID]
	
});

	