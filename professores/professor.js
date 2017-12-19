var dataJson = {
	"n_dias_com_aula": 5,
	"n_horarios_por_dia": 10,
	"n_turmas": 10,
	"n_professores": 19,
	"n_turnos": 2,
	"n_materias": 8,
	"quadras": 1,
	"materias": [
		"Mat",
		"Prt",
		"Cie",
		"His",
		"Geo",
		"Ing",
		"EdF",
		"Art"
	],
	"aula_professor_turma": [
		[3, 3, 3, 3, 3, 0, 3, 3, 0, 3],
		[2, 2, 0, 2, 0, 2, 0, 2, 2, 0],
		[0, 0, 0, 0, 3, 0, 0, 3, 3, 3],
		[2, 0, 2, 2, 0, 2, 2, 0, 0, 2],
		[0, 0, 0, 0, 0, 0, 0, 6, 6, 6],
		[0, 0, 0, 0, 0, 6, 6, 0, 0, 0],
		[2, 2, 2, 0, 2, 0, 2, 2, 0, 0],
		[3, 0, 3, 0, 0, 0, 0, 3, 3, 0],
		[0, 0, 0, 0, 0, 3, 0, 0, 3, 0],
		[0, 6, 0, 0, 0, 4, 4, 4, 4, 4],
		[0, 0, 0, 0, 6, 0, 0, 0, 0, 0],
		[3, 3, 3, 3, 0, 0, 0, 0, 0, 0],
		[0, 0, 0, 2, 0, 2, 0, 0, 2, 2],
		[6, 0, 6, 6, 4, 0, 0, 0, 0, 0],
		[4, 4, 4, 4, 0, 0, 0, 0, 0, 0],
		[0, 3, 0, 3, 3, 3, 3, 0, 0, 3],
		[0, 2, 0, 0, 2, 0, 0, 2, 2, 0],
		[0, 0, 2, 0, 2, 0, 2, 0, 0, 2],
		[0, 0, 0, 0, 0, 3, 3, 0, 0, 0]
	],
	"professor_disciplina": [
		3, 6, 2, 7, 0, 0, 5, 4, 3, 1, 0, 2, 5, 1, 0, 4, 7, 6, 2
	],
	"turma_turno": [
		0, 1, 0, 1, 0, 1, 1, 0, 0, 1
	],
	"turma_nome": [
		"1701", "1702", "1703", "1704", "1801", "1802", "1803", "1901", "1902", "1903"
	],
	"horarios": [
		"07:30", "08:20", "09:10", "10:20", "11:10", "13:00", "13:50", "15:00", "15:50", "16:40"
	]
};


function insertIntoDiv(data) {
	document.getElementById('div_data').innerHTML += data;
}
// var mydata = JSON.parse(data);

function returnDiaSemana(number){
	if (number == 0) {
		return "Segunda";
	}
	else if (number == 1) {
		return "Terca";
	}
	else if (number == 2) {
		return "Quarta";
	}
	else if (number == 3) {
		return "Quinta";
	}
	else if (number == 4) {
		return "Sexta";
	}
}

for (var i = 0; i < dataJson.n_horarios_por_dia; i++) {
	document.getElementById('div_horario').innerHTML += '<span style="position: absolute;left: '+(((i+1)*100) - 30)+'px;">Horario [' + (i+1) + '] </span>';
}

for (var i = 0; i < dataJson.n_dias_com_aula; i++) {
	var diaSemana = returnDiaSemana(i);
	insertIntoDiv("<div id='dia_name'> " + diaSemana + " <div>");


	
	for (var j = 0; j < dataJson.n_horarios_por_dia; j++) {

		var valor_check = i + "_" + j;

		insertIntoDiv('<input style="position: absolute;left: '+(j+1)*100+'px;" type="checkbox" id='+valor_check+' name='+valor_check+' value='+valor_check+'>');
		
	}

	insertIntoDiv("<br/><br/>");
}

var professor_matrix = new Array(dataJson.n_dias_com_aula);
for (var i = 0; i < dataJson.n_dias_com_aula; i++) {
  professor_matrix[i] = new Array(dataJson.n_horarios_por_dia);
}


function download(text, name, type) {
    var a = document.createElement("a");
    var file = new Blob([text], {type: type});
    a.href = URL.createObjectURL(file);
    a.download = name;
    a.click();
}

function Salvar() {
	for (var i = 0; i < dataJson.n_dias_com_aula; i++) {
	
		for (var j = 0; j < dataJson.n_horarios_por_dia; j++) {

			var valor_check = i + "_" + j;

			if (document.getElementById(valor_check).checked == true) {
				professor_matrix[i][j] = 1;
			}else{
				professor_matrix[i][j] = 0;

			}


		}
	}

	for (var i = 0; i < dataJson.n_dias_com_aula; i++) {
	
		for (var j = 0; j < dataJson.n_horarios_por_dia; j++) {

			document.getElementById('div_result').innerHTML += '<span >[' + professor_matrix[i][j] + '] </span>';

		}
		document.getElementById('div_result').innerHTML += '<br/>';

	}

	download(professor_matrix, 'test.json', 'data:text/json;charset=utf-8');


}








// alert("dasdsa");