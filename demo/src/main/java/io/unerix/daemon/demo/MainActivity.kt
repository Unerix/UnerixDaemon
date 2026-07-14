package io.unerix.daemon.demo

import android.os.Bundle
import androidx.activity.ComponentActivity
import androidx.activity.compose.setContent
import androidx.activity.enableEdgeToEdge
import androidx.compose.foundation.layout.BoxWithConstraints
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.layout.size
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.Scaffold
import androidx.compose.material3.Surface
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.tooling.preview.Preview
import androidx.compose.ui.unit.dp
import androidx.compose.ui.viewinterop.AndroidView
import io.unerix.daemon.UnerixView
import io.unerix.daemon.demo.ui.theme.UnerixDaemonTheme

class MainActivity : ComponentActivity() {
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        enableEdgeToEdge()
        setContent {
            UnerixDaemonTheme {
                Scaffold(modifier = Modifier.fillMaxSize()) { innerPadding ->
                    BoxWithConstraints(
                        modifier = Modifier
                            .fillMaxSize()
                            .padding(paddingValues = innerPadding)
                            .padding(all = 16.dp),
                        contentAlignment = Alignment.Center,
                    ) {
                        val aspectRatio = 480f / 320f
                        val (width, height) = if (maxWidth / maxHeight > aspectRatio) {
                            maxHeight * aspectRatio to maxHeight
                        } else {
                            maxWidth to maxWidth / aspectRatio
                        }
                        Surface(
                            modifier = Modifier.size(width, height),
                            color = MaterialTheme.colorScheme.primaryContainer,
                            shape = MaterialTheme.shapes.large,
                        ) {
                            AndroidView(
                                factory = { context -> UnerixView(context) },
                                modifier = Modifier.fillMaxSize(),
                            )
                        }
                    }
                }
            }
        }
    }
}

@Composable
fun Greeting(name: String, modifier: Modifier = Modifier) {
    Text(
        text = "Hello $name!",
        modifier = modifier
    )
}

@Preview(showBackground = true)
@Composable
fun GreetingPreview() {
    UnerixDaemonTheme {
        Greeting("Android")
    }
}