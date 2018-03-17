$(document).ready(function() {

    namespace = '/biocl';

    // Connect to the Socket.IO server.
    var socket = io.connect(location.protocol + '//' +
	    		       document.domain + ':' +
		             location.port + namespace);



    //Se escuchan las mediciones de ph, OD, Temp.
    socket.on('Medidas', function(msg) {
        $('#med1').text('pH: '    + msg.data[0]          ).html();
        $('#med2').text('OD: '    + msg.data[1] + ' %'   ).html();
        $('#med3').text('Temp: '  + msg.data[2] + ' º[C]').html();
    });

    //se emiten los setpoints hacia el servidor
    $('form#setpoints').submit(function(event) {
        socket.emit('Setpoints',
                    { alimentar:     $('#alimentar').val(),
                        mezclar:     $('#mezclar').val(),
                             ph:     $('#ph').val(),
                       descarga:     $('#descargar').val(),
                    temperatura:     $('#temperatura').val(),
                      alimentar_rst: $('#alimentar_rst').is(':checked'),  //rst1
                        mezclar_rst: $('#mezclar_rst').is(':checked'),    //rst2
                             ph_rst: $('#ph_rst').is(':checked'),         //rst3
                       descarga_rst: $('#descargar_rst').is(':checked'),  //rst4
                    temperatura_rst: $('#temperatura_rst').is(':checked'),//rst5
                      alimentar_dir: $('#alimentar_dir').is(':checked'),  //dir1
                             ph_dir: $('#ph_dir').is(':checked'),         //dir2
                    temperatura_dir: $('#temperatura_dir').is(':checked') //dir3
                     });

        //para depurar
        console.log('Emitiendo Valores: alimentar, mezclar, ph, descargar, temperatura:');
        console.log($('#alimentar').val());
        console.log($('#mezclar').val());
        console.log($('#ph').val());
        console.log($('#descargar').val());
        console.log($('#temperatura').val());

        console.log('Emitiendo Chekeds');
        console.log($('#alimentar_rst').is(':checked'));
        console.log($('#mezclar_rst').is(':checked'));
        console.log($('#ph_rst').is(':checked'));
        console.log($('#descargar_rst').is(':checked'));
        console.log($('#temperatura_rst').is(':checked'));
        console.log($('#alimentar_dir').is(':checked'));
        console.log($('#ph_dir').is(':checked'));
        console.log($('#temperatura_dir').is(':checked'));

        return false;
    });

    //se escuchan desde el servidor los setpoints aplicados
    //para ser desplegados en todos los clientes.
    socket.on('Setpoints', function(msg) {
        document.getElementById('alimentar').value         = msg.set[0];
        document.getElementById('mezclar').value           = msg.set[1];
        document.getElementById('ph').value                = msg.set[2];
        document.getElementById('descargar').value         = msg.set[3];
        document.getElementById('temperatura').value       = msg.set[4];
        document.getElementById('alimentar_rst').checked   = msg.set[5];
        document.getElementById('mezclar_rst').checked     = msg.set[6];
        document.getElementById('ph_rst').checked          = msg.set[7];
        document.getElementById('descargar_rst').checked   = msg.set[8];
        document.getElementById('temperatura_rst').checked = msg.set[9];
        document.getElementById('alimentar_dir').checked   = msg.set[10];
        document.getElementById('ph_dir').checked          = msg.set[11];
        document.getElementById('temperatura_dir').checked = msg.set[12];

        //para depurar
        console.log('Checkeds Recibidos');
        console.log($('#alimentar_rst').is(':checked'));
        console.log($('#mezclar_rst').is(':checked'));
        console.log($('#ph_rst').is(':checked'));
        console.log($('#descargar_rst').is(':checked'));
        console.log($('#temperatura_rst').is(':checked'));
        console.log($('#alimentar_dir').is(':checked'));
        console.log($('#ph_dir').is(':checked'));
        console.log($('#temperatura_dir').is(':checked'));

    });






    //se emiten señal de reinicio,apagado, grabacion y limpiaza hacia el servidor
    $('form#process').submit(function(event) {
        socket.emit('power',
                    { action  : $('select[name=selection]').val(),
                      checked : $('#confirm').is(':checked')
                   });

        //para depurar
        console.log('Emitiendo Valores de Acción');
        console.log($('select[name=selection]').val())
        console.log($('#confirm').is(':checked'));

        return false;
    });

    //se escuchan desde el servidor señal de reinicio,apagado, grabacion y limpiaza
    //para ser desplegados en todos los clientes.
    socket.on('power', function(msg) {
        document.getElementById("select").value = msg.set[0]
        document.getElementById('confirm').checked = msg.set[1]

        console.log('Recibiendo Valores de Acción');
        console.log(msg.set[0]);
        console.log(msg.set[1]);
    });


});
