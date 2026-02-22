import 'package:flutter/material.dart';
import '../services/dispenser_service.dart';


class ConfigurarHorario extends StatefulWidget
{
  const ConfigurarHorario({super.key});

  @override
  State<ConfigurarHorario> createState() => _ConfigurarHorarioState();
}

class _ConfigurarHorarioState extends State<ConfigurarHorario>
{
  // Periódico
  final TextEditingController _horasCtrl = TextEditingController();
  final TextEditingController _minutosCtrl = TextEditingController();

  // Único
  final TextEditingController _horaUnicaCtrl = TextEditingController();
  final TextEditingController _minutoUnicoCtrl = TextEditingController();

  @override
  void dispose()
  {
    _horasCtrl.dispose();
    _minutosCtrl.dispose();
    _horaUnicaCtrl.dispose();
    _minutoUnicoCtrl.dispose();
    super.dispose();
  }

  Future<void> _confirmarPeriodico()
  async
  {
    final int horas =
        int.tryParse(_horasCtrl.text.trim().isEmpty ? '0' : _horasCtrl.text) ?? 0;
    final int minutos =
        int.tryParse(_minutosCtrl.text.trim().isEmpty ? '0' : _minutosCtrl.text) ?? 0;

    if (horas == 0 && minutos == 0)
    {
      Navigator.pop(context, {'cancelado': true});
      return;
    }

    final bool ok = await DispenserService.programarCada(
      horas: horas,
      minutos: minutos,
    );

    if (!mounted) return;

    if (ok)
    {
      Navigator.pop(context,
          {
            'tipo': 'periodico',
            'horas': horas,
            'minutos': minutos,
          });
    }
    else
    {
      ScaffoldMessenger.of(context).showSnackBar(
        const SnackBar(
          content: Text('Error: no se pudo programar el dispensado periódico'),
        ),
      );
    }
  }

  Future<void> _cancelarPeriodico()
  async
  {
    final bool ok = await DispenserService.programarCada(
      horas: 0,
      minutos: 0,
    );

    if (!mounted) return;

    if (ok)
    {
      Navigator.pop(context, {'cancelado': true});
    }
    else
    {
      ScaffoldMessenger.of(context).showSnackBar(
        const SnackBar(
          content: Text('Error: no se pudo cancelar el dispensado periódico'),
        ),
      );
    }
  }



  Future<void> _confirmarUnico()
  async
  {
    final int hora =
        int.tryParse(_horaUnicaCtrl.text.trim().isEmpty ? '0' : _horaUnicaCtrl.text) ?? 0;
    final int minuto =
        int.tryParse(_minutoUnicoCtrl.text.trim().isEmpty ? '0' : _minutoUnicoCtrl.text) ?? 0;

    final bool ok = await DispenserService.programarUnico(hora: hora, minuto: minuto);

    if (!mounted) return;

    if (ok)
    {
      Navigator.pop(context,
          {
            'tipo': 'unico',
            'hora': hora,
            'minuto': minuto,
          });
    }
    else
    {
      ScaffoldMessenger.of(context).showSnackBar(
        const SnackBar(content: Text('Error: no se pudo contactar al dispenser (192.168.4.1)')),
      );
    }
  }

  Future<void> _cancelarUnico()
  async
  {
    final bool ok = await DispenserService.programarUnico(
      hora: 0,
      minuto: 0,
    );

    if (!mounted) return;

    if (ok)
    {
      Navigator.pop(context, {'cancelado': true});
    }
    else
    {
      ScaffoldMessenger.of(context).showSnackBar(
        const SnackBar(
          content: Text('Error: no se pudo cancelar el dispensado único'),
        ),
      );
    }
  }


  @override
  Widget build(BuildContext context)
  {
    const TextStyle estiloTitulo = TextStyle(
      fontSize: 16,
      fontWeight: FontWeight.w600,
    );

    return Scaffold(
      appBar: AppBar(
        title: const Text('Configurar tiempo'),
      ),
      body: SingleChildScrollView(
        padding: const EdgeInsets.all(20),
        child: Column(
          crossAxisAlignment: CrossAxisAlignment.stretch,
          children: <Widget>[

            // =======================
            // DISPENSADO PERIÓDICO
            // =======================
            const Text('Dispensar cada:', style: estiloTitulo),
            const SizedBox(height: 12),

            Row(
              children: <Widget>[
                Expanded(
                  child: TextField(
                    controller: _horasCtrl,
                    keyboardType: TextInputType.number,
                    decoration: const InputDecoration(
                      labelText: 'Horas',
                      border: OutlineInputBorder(),
                    ),
                  ),
                ),
                const SizedBox(width: 12),
                Expanded(
                  child: TextField(
                    controller: _minutosCtrl,
                    keyboardType: TextInputType.number,
                    decoration: const InputDecoration(
                      labelText: 'Minutos',
                      border: OutlineInputBorder(),
                    ),
                  ),
                ),
              ],
            ),

            const SizedBox(height: 12),

            ElevatedButton(
              style: ElevatedButton.styleFrom(
                backgroundColor: Colors.green,
                foregroundColor: Colors.white,
              ),
              onPressed: _confirmarPeriodico,
              child: const Text('Confirmar dispensado periódico'),
            ),

            const SizedBox(height: 8),

            ElevatedButton(
              style: ElevatedButton.styleFrom(
                backgroundColor: Colors.red,
                foregroundColor: Colors.white,
              ),
              onPressed: _cancelarPeriodico,
              child: const Text('Cancelar dispensado periódico'),
            ),


            const SizedBox(height: 32),
            const Divider(),
            const SizedBox(height: 24),

            // =======================
            // DISPENSADO ÚNICO
            // =======================
            const Text('Programar dispensado único:', style: estiloTitulo),
            const SizedBox(height: 12),

            Row(
              children: <Widget>[
                Expanded(
                  child: TextField(
                    controller: _horaUnicaCtrl,
                    keyboardType: TextInputType.number,
                    decoration: const InputDecoration(
                      labelText: 'Hora',
                      border: OutlineInputBorder(),
                    ),
                  ),
                ),
                const SizedBox(width: 12),
                Expanded(
                  child: TextField(
                    controller: _minutoUnicoCtrl,
                    keyboardType: TextInputType.number,
                    decoration: const InputDecoration(
                      labelText: 'Minuto',
                      border: OutlineInputBorder(),
                    ),
                  ),
                ),
              ],
            ),

            const SizedBox(height: 12),

            ElevatedButton(
              style: ElevatedButton.styleFrom(
                backgroundColor: Colors.green,
                foregroundColor: Colors.white,
              ),
              onPressed: _confirmarUnico,
              child: const Text('Confirmar dispensado único'),
            ),

            const SizedBox(height: 8),

            ElevatedButton(
              style: ElevatedButton.styleFrom(
                backgroundColor: Colors.red,
                foregroundColor: Colors.white,
              ),
              onPressed: _cancelarUnico,
              child: const Text('Cancelar dispensado único'),
            ),
          ],
        ),
      ),
    );
  }
}
