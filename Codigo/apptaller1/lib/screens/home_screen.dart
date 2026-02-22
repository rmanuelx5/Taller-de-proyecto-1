import 'package:flutter/material.dart';
import 'configurar_horario.dart';
import '../services/dispenser_service.dart';


class HomeScreen extends StatefulWidget
{
  const HomeScreen({super.key});

  @override
  State<HomeScreen> createState() => _HomeScreenState();
}

class _HomeScreenState extends State<HomeScreen>
{
  String _resumenIntervalo = 'Sin programación';

  @override
  Widget build(BuildContext context)
  {
    final ButtonStyle estiloComun = ElevatedButton.styleFrom(
      minimumSize: const Size(220, 48),
      overlayColor: Colors.black12,
      shape: RoundedRectangleBorder(borderRadius: BorderRadius.circular(10)),
    );

    return Scaffold(
      appBar: AppBar(
        title: const Text('Dispenser de comida'),
        centerTitle: true,
      ),
      body: Center(
        child: Column(
          mainAxisAlignment: MainAxisAlignment.center,
          children: <Widget>[

            // Estado visible del intervalo elegido
            Padding(
              padding: const EdgeInsets.only(bottom: 24),
              child: Text(
                'Intervalo: $_resumenIntervalo',
                style: Theme.of(context).textTheme.titleMedium,
              ),
            ),

            // Botón verde: expulsar alimento ahora
            ElevatedButton(
              style: ElevatedButton.styleFrom(
                minimumSize: const Size(240, 56),
                backgroundColor: Colors.green,
                foregroundColor: Colors.white,
                overlayColor: Colors.white24,
                padding: const EdgeInsets.symmetric(horizontal: 40, vertical: 18),
                shape: RoundedRectangleBorder(borderRadius: BorderRadius.circular(12)),
              ),
              onPressed: () async
              {
                ScaffoldMessenger.of(context).showSnackBar(
                  const SnackBar(content: Text('Enviando comando al dispenser…')),
                );

                final bool ok = await DispenserService.dispensarAhora();

                if (!mounted) return;

                if (ok)
                {
                  ScaffoldMessenger.of(context).showSnackBar(
                    const SnackBar(content: Text('Dispensado solicitado ✅')),
                  );
                }
                else
                {
                  ScaffoldMessenger.of(context).showSnackBar(
                    const SnackBar(content: Text('Error: no se pudo contactar al dispenser (192.168.4.1)')),
                  );
                }
              },
              child: const Text('Expulsar alimento', style: TextStyle(fontSize: 18)),
            ),

            const SizedBox(height: 20),

            // Botón para configurar intervalo
            ElevatedButton(
              style: estiloComun,
              onPressed: () async
              {
                final resultado = await Navigator.push(
                  context,
                  MaterialPageRoute(
                    builder: (context) => const ConfigurarHorario(),
                  ),
                );

                if (!mounted) return;

                if (resultado is Map)
                {
                  // Caso cancelar (o confirmar con 0/0 desde la otra pantalla)
                  if (resultado['cancelado'] == true)
                  {
                    setState(()
                    {
                      _resumenIntervalo = 'Sin programación';
                    });
                  }
                  // Caso confirmar con horas/minutos
                  else if (resultado['horas'] != null && resultado['minutos'] != null)
                  {
                    final int h = resultado['horas'] as int;
                    final int m = resultado['minutos'] as int;

                    // Si ambos son 0, también consideramos sin programación
                    if (h == 0 && m == 0)
                    {
                      setState(()
                      {
                        _resumenIntervalo = 'Sin programación';
                      });
                    }
                    else
                    {
                      setState(()
                      {
                        _resumenIntervalo = '$h h $m min';
                      });

                      ScaffoldMessenger.of(context).showSnackBar(
                        SnackBar(content: Text('Intervalo guardado: $_resumenIntervalo')),
                      );
                    }
                  }
                }
              },
              child: const Text('Configurar tiempo'),
            ),
          ],
        ),
      ),
    );
  }
}
